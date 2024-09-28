#include <lualib.h>
#include <lua.h>
#include <Offsets.hpp>


#include <Shlwapi.h>
#define _WIN32_WINNT 0x0500  // Or a higher version like 0x0600 for newer features
#include <windows.h>
#define ARG_CHECK(L, MIN, MAX, T) if (lua_gettop(L) < MIN) { T return 0; } else if (lua_gettop(L) > MAX) { lua_settop(L, MAX); } T
#include <shlobj.h>  // Required for SHGetFolderPath
#include <VM/src/ldebug.h>
std::vector<std::string> disallowed_extensions = {
            ".exe", ".bat", ".com", ".csh", ".msi", ".vb", ".vbs", ".vbe", ".ws", ".wsf", ".wsh", ".ps1"
};
auto equals_ignore_case(std::string_view a, std::string_view b) -> bool {
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        }
    );
}





std::string getAppDataPath() {
    char appDataPath[MAX_PATH];

    // Get the AppData path
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        // Construct the full path to the "Fent" folder
        std::string fentPath = std::string(appDataPath) + "\\Fent";

        // Create the "Fent" folder if it doesn't exist
        if (CreateDirectoryA(fentPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            // Return the full path to the "Fent" folder
            return fentPath;
        }
        else {
            // Failed to create directory
            std::cerr << "Failed to create 'Fent' folder. Error code: " << GetLastError() << std::endl;
            return "";
        }
    }

    // Failed to get AppData path
    return "";
}
auto read_file(std::string file_location) -> std::string {
    auto close_file = [](FILE* f) { fclose(f); };
    auto holder = std::unique_ptr<FILE, decltype(close_file)>(fopen(file_location.c_str(), "rb"), close_file);

    if (!holder)
        return "";

    FILE* f = holder.get();

    if (fseek(f, 0, SEEK_END) < 0)
        return "";

    const long size = ftell(f);

    if (size < 0)
        return "";

    if (fseek(f, 0, SEEK_SET) < 0)
        return "";

    std::string res;
    res.resize(size);
    fread(const_cast<char*>(res.data()), 1, size, f);

    return res;
}
int lua_readfile(lua_State* L) {
    // Get the filename from Lua
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);

    

    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);
    RBX::Print(1, path.c_str());
    //std::cout << "reading at - " << path << "\n";

    if (!std::filesystem::exists(path.c_str())) {
        //std::cout << "file does not exist!\n";
        luaL_error(L, "file does not exist");
        return 0;
    }

    std::string output = read_file(path);
    lua_pushlstring(L, output.c_str(), output.size());
    return 1;
}

// Function to write to a file in AppData
int lua_writefile(lua_State* L) {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING);)
        luaL_checktype(L, 2, LUA_TSTRING);

    std::string path = lua_tostring(L, 1);
    std::string content = lua_tostring(L, 2);

    // Get the file extension
    const std::string extension = PathFindExtensionA(path.c_str());

    // Check for disallowed extensions
    for (const std::string& test : disallowed_extensions) {
        if (equals_ignore_case(extension, test)) {
            luaL_error(L, "disallowed file extension");
            return 0;
        }
    }

    // Prevent directory traversal
    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    // Construct the full path in the AppData folder
    path = (getAppDataPath() + "\\" + path);
    RBX::Print(1, path.c_str());

    // Create the directory if it doesn't exist
    std::filesystem::path filePath(path);
    std::filesystem::create_directories(filePath.parent_path());

    // Write the content to the file
    std::ofstream out(filePath);
    out << content;
    out.close();

    return 0;
}

auto lua_isfolder(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);


    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    lua_pushboolean(L, std::filesystem::is_directory(path));
    return 1;
}
auto lua_makefolder(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);

    

    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);
    RBX::Print(1, path.c_str());
    std::filesystem::create_directories(path);
   // std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
auto lua_isfile(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);



    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    lua_pushboolean(L, std::filesystem::is_regular_file(path));
    return 1;
}
auto replace(std::string subject, std::string search, std::string replace) -> std::string {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }

    return subject;
};
auto lua_listfiles(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);



    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    if (!std::filesystem::is_directory(path.c_str())) {
        luaL_error(L, "directory does not exist");
        return 0;
    }

    lua_newtable(L);

    int idx = 0;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        auto file_path = entry.path().string();
        file_path = replace(file_path, (getAppDataPath() + "\\"), "");

        lua_pushinteger(L, ++idx);
        lua_pushstring(L, file_path.c_str());
        lua_settable(L, -3);
    }

    return 1;
}
auto lua_delfolder(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);


    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    if (!std::filesystem::remove_all(path)) {
        luaL_error(L, "folder does not exist");
        return 0;
    }

    return 0;
}

auto lua_delfile(lua_State* L) -> int {
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
        std::string path = lua_tostring(L, 1);



    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    if (!std::filesystem::remove(path)) {
        luaL_error(L, "file does not exist");
        return 0;
    }

    return 0;
}


auto lua_appendfile(lua_State* L) -> int {
    ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING);)
        luaL_checktype(L, 2, LUA_TSTRING);

    std::string path = lua_tostring(L, 1);
    std::string content = lua_tostring(L, 2);

    const std::string extention = PathFindExtensionA(path.c_str());
    for (std::string& test : disallowed_extensions) {
        if (equals_ignore_case(extention, test)) {
            luaL_error(L, "attempt to escape directory");
            return 0;
        }
    }

    if (path.find("..") != std::string::npos) {
        luaL_error(L, "attempt to escape directory");
        return 0;
    }

    path = (getAppDataPath() + "\\" + path);

    std::ofstream out;
    out.open(path, std::ios_base::app | std::ios_base::binary);
    out.write(content.c_str(), content.size());
    out.close();

    return 0;
}

std::string getExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);

    std::wstring exePath(buffer);
    std::wstring::size_type pos = exePath.find_last_of(L"\\/"); // Find last backslash
    exePath = exePath.substr(0, pos); // Extract path before the executable

    // Convert to string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, exePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string exePathString(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, exePath.c_str(), -1, &exePathString[0], size_needed, nullptr, nullptr);

    // Remove the null terminator added by WideCharToMultiByte
    exePathString.resize(size_needed - 1);

    return exePathString;
}

int getcustomasset(lua_State* L) {
    // Check the Lua argument is a string
    ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););

    // Get the current directory
    wchar_t buffer[MAX_PATH];
    DWORD length = GetCurrentDirectory(MAX_PATH, buffer);
    if (length == 0) {
        RBX::Print(1, "Failed to get current directory.");
        return 1;
    }
    std::string exePath = std::filesystem::path(buffer).string();

    // Append the content directory to the current directory
    std::string contentDir = exePath + "\\content\\BA1C3BA115S";

    RBX::Print(1, "Exe Path: %s", exePath.c_str());
    RBX::Print(1, "Content Directory: %s", contentDir.c_str());

    // Check if the directory already exists, and create it if not
    if (!std::filesystem::exists(contentDir)) {
        std::filesystem::create_directory(contentDir);
        RBX::Print(1, "Directory created: %s", contentDir.c_str());
    }
    else {
        RBX::Print(1, "Directory already exists: %s", contentDir.c_str());
    }

    // Prepare the source path from AppData and Lua input
    std::string workspacePath = getAppDataPath() + "\\" + lua_tostring(L, 1);
    RBX::Print(1, "Workspace path: %s", workspacePath.c_str());

    // Check if the source file exists
    if (!std::filesystem::exists(workspacePath)) {
        RBX::Print(1, "Source file does not exist: %s", workspacePath.c_str());
        return 1; // Return early if the source file doesn't exist
    }

    // Define the target path for copying the file
    std::string customAssetFile = contentDir + "\\" + std::filesystem::path(workspacePath).filename().string();
    RBX::Print(1, "Copying to: %s", customAssetFile.c_str());

    // Copy the file, updating it if it already exists
    std::filesystem::copy_file(workspacePath, customAssetFile, std::filesystem::copy_options::update_existing);

    // Push the asset URL to Lua
    lua_pushstring(L, std::string("rbxasset://BA1C3BA115S/" + std::filesystem::path(customAssetFile).filename().string()).c_str());

    return 1; // Return success
}
int loadstring(lua_State* L, const std::string& code) {

    static auto Encoder = CBytecodeEncoder();
    auto Bytecode = Luau::compile(code, { 2, 1, 2 }, { true, true }, &Encoder);

    if (luau_load(L, "goodexec", Bytecode.c_str(), Bytecode.size(), 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        RBX::Print(1, "[Execution::lua_loadstring] luau_load failed with error \'%s\'", err);
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushlstring(L, err, strlen(err));
        return 2;
    }

    return 1;
}
int loadfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);



    if (path.find("..") != std::string::npos)
        luaG_runerror(L, "attempt to escape directory");

    path = (getAppDataPath() + "\\" + path);

    if (!std::filesystem::exists(path.c_str()))
        luaG_runerror(L, "file does not exist");

    std::string output = read_file(path);

    return loadstring(L, output);
}


static const luaL_Reg env_funcs[] = {
    {"getcustomasset", getcustomasset},
    {"readfile", lua_readfile},
   {"loadfile", loadfile},
    {"writefile", lua_writefile},
    {"makefolder", lua_makefolder},
    {"isfolder", lua_isfolder},
    {"isfile", lua_isfile},
    {"delfolder", lua_delfolder},
    {"listfiles", lua_listfiles},
    {"appendfile", lua_appendfile},
    {"delfile", lua_delfile},
    {NULL, NULL}  // Sentinel to mark the end of the array
};
// Function to register C++ functions in Luau
void register_messagebox(lua_State* L) {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, NULL, env_funcs);
    lua_pop(L, 1);

}