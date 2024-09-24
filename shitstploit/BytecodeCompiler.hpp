#include <Compiler/src/Builtins.h>
#include "Dependencies/zstd/zstd.h"
#include <stdexcept>
#include "Dependencies/zstd/common/xxhash.h"
#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>
#include <Luau/BytecodeUtils.h>
class bytecode_encoder_t : public Luau::BytecodeEncoder {
    inline void encode(uint32_t* data, size_t count) override {
        for (auto i = 0u; i < count;) {
            auto& opcode = *reinterpret_cast<uint8_t*>(data + i);
            i += Luau::getOpLength(LuauOpcode(opcode));

            opcode *= 227;
        }
    }
};

bytecode_encoder_t bytecoder_encoder = {};
Luau::CompileOptions compile_options;

std::string compress(const std::string& bytecode, size_t& byte_size) {

    const auto data_size = bytecode.size();
    const auto max_size = ZSTD_compressBound(data_size);
    std::string buffer(max_size + 8, '\0'); // Initialize with null characters

    // Add header "RSB1" and data_size to the beginning of the buffer
    buffer.replace(0, 4, "RSB1H");
    std::memcpy(&buffer[4], &data_size, sizeof(data_size));

    // Compress the data
    const auto compressed_size = ZSTD_compress(&buffer[8], max_size, bytecode.data(), data_size, ZSTD_maxCLevel());
    if (ZSTD_isError(compressed_size)) {
        throw std::runtime_error("Failed to compress the bytecode.");
    }

    // Adjust the buffer size to the actual compressed data size plus header
    buffer.resize(compressed_size + 8);

    // Encrypt the buffer
    const auto size = compressed_size + 8;
    const auto key = XXH32(buffer.data(), size, 42u);
    const auto bytes = reinterpret_cast<const uint8_t*>(&key);

    for (size_t i = 0; i < size; ++i) {
        buffer[i] ^= bytes[i % 4] + i * 41u;
    }

    byte_size = size;

    return buffer;
}

std::string compile(std::string source) {
    if (compile_options.debugLevel != 2) {
        compile_options.debugLevel = 2;
        compile_options.optimizationLevel = 2;
    }

    std::string bytecode = Luau::compile(
        source,
        compile_options,
        {}, &bytecoder_encoder
    );

    return bytecode;
}