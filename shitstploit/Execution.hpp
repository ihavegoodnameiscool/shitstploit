#include <Windows.h>
#include <iostream>
#include "Update/Offsets.hpp"
#include <string>
#include <thread>
#include <sstream>
#include <cstring>
#include "BytecodeCompiler.hpp";
#include <tuple>
#include "DataModelFetcher.hpp"
#include <lualib.h>


uintptr_t DataModel(uintptr_t view_address) {
    return *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(view_address + 0x118) + 0x198);
}

uintptr_t ScriptContext(uintptr_t DataModel) {
    return *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(DataModel + 80) + 0x0) + (976));
}

uintptr_t Identity(uintptr_t lua_state) {
    return *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(lua_state + 120) + 48);
}

uintptr_t UserData(uintptr_t lua_state) {
    return lua_state + 120;
}

const char LocalPlayer(uintptr_t DataModel) {
    return *reinterpret_cast<const char*>(DataModel + 0x108);
}

const char LocalPlayerName(uintptr_t LocalPlayer) {
    return *reinterpret_cast<const char*>(LocalPlayer + 0x50);
}

class CBytecodeEncoder : public Luau::BytecodeEncoder
{
    inline void encode(uint32_t* Data, size_t Count) override
    {
        for (auto i = 0u; i < Count;)
        {
            auto& Opcode = *(uint8_t*)(Data + i);
            i += Luau::getOpLength(LuauOpcode(Opcode));
            Opcode *= 227;
        }
    }
};



namespace StateTools
{
    uintptr_t state;
    void SetCapabilities(Proto* Proto, uintptr_t* CapFlags)
    {
        if (!Proto)
            return;

        Proto->userdata = CapFlags;
        for (auto i = 0; i < Proto->sizep; ++i)
            SetCapabilities(Proto->p[i], CapFlags);
    }

    uintptr_t getLuaState() {
        uintptr_t view_address = GetRenderView();
        RBX::Print(1, ("View Address: " + std::to_string(view_address)).c_str());
        uintptr_t dataModel = DataModel(view_address);
        RBX::Print(1, ("DataModel Address: " + std::to_string(dataModel)).c_str());
        uintptr_t scriptContext = ScriptContext(dataModel);
        RBX::Print(1, ("Script Context Address: " + std::to_string(view_address)).c_str());
        const auto EncryptedState = *(uintptr_t*)(*(uintptr_t*)(*(uintptr_t*)(dataModel + 0x50) + 0x0) + 0x3D0) + 0x1C8;
        RBX::Print(1, ("Encrypted Lua State: " + std::to_string(EncryptedState)).c_str());
        auto LuaState = (uintptr_t)lua_newthread((lua_State*)(EncryptedState + *(int*)EncryptedState));
        RBX::Print(1, ("Decrypted Lua State: " + std::to_string(LuaState)).c_str());
        RBX::Print(1, "FentanylSploit finished initalising");
        state = LuaState;
        return LuaState;
    }
    static uintptr_t MaxCapabilities = 0xEFFFFFFFFFFFFFFF;

    void Execute(const std::string& Source, uintptr_t Lua_State) {
        RBX::Print(1, "Execute Called");
        if (Source.empty())
            return;

        const auto MainThread = (lua_State*)Lua_State;
        auto ExecutionThread = lua_newthread(MainThread);
  
        static auto Encoder = CBytecodeEncoder();
        auto Bytecode = Luau::compile(Source, {2, 1, 2}, {true, true}, &Encoder);
        if (Bytecode[0] != '\0')
        {
            const auto ExtraSpace = (uintptr_t)ExecutionThread->userdata;
            *(__int64*)(ExtraSpace + Update::Userdata::Identity) = 8;
            *(__int64*)(ExtraSpace + Update::Userdata::Capabilities) = MaxCapabilities;
   
       
            lua_getglobal(ExecutionThread, "task");
            lua_getfield(ExecutionThread, -1, "defer");

            if (luau_load(ExecutionThread, "ihavegoodexecutor", Bytecode.c_str(), Bytecode.size(), 0) != LUA_OK)
            {
                const auto Error = lua_tostring(ExecutionThread, -1);
                lua_getglobal(ExecutionThread, "error");
                lua_pushstring(ExecutionThread, Error);
                lua_call(ExecutionThread, 1, 0);
                return;
            }

            SetCapabilities(((Closure*)lua_topointer(ExecutionThread, -1))->l.p, &MaxCapabilities);

            if (lua_pcall(ExecutionThread, 1, 0, 0) != LUA_OK)
            {
                const auto Error = lua_tostring(ExecutionThread, -1);
                lua_getglobal(ExecutionThread, "error");
                lua_pushstring(ExecutionThread, Error);
                lua_call(ExecutionThread, 1, 0);
            }
            return;
        }

        lua_getglobal(ExecutionThread, "error");
        lua_pushstring(ExecutionThread, Bytecode.c_str() + 1);
        lua_call(ExecutionThread, 1, 0);
    }


}
