#pragma once
#include <Windows.h>
#include <iostream>

#define REBASE(x) x + (uintptr_t)GetModuleHandle(nullptr)

#define RBX_PRINT_NORMAL 0
#define RBX_PRINT_INFO 1
#define RBX_PRINT_WARN 2
#define RBX_PRINT_ERROR 3

struct lua_State;

namespace Update
{
	const uintptr_t Print = REBASE(0x11FE010);
	const uintptr_t GetTaskScheduler = REBASE(0x2B40B90);
	const uintptr_t TaskDefer = REBASE(0xDF1E20);
	const uintptr_t LuaVMLoad = REBASE(0xA88610);
	const uintptr_t DecryptState = REBASE(0xA86050);

	const uintptr_t PushInstance = REBASE(0xC78970);
	const uintptr_t LuaO_NilObject = REBASE(0x4293D98);
	const uintptr_t Luau_Execute = REBASE(0x2510DA0);
	const uintptr_t LuaH_DummyNode = REBASE(0x4293868);
	

	namespace TaskScheduler
	{
		const uintptr_t JobsStart = 0x198;
		const uintptr_t JobsEnd = 0x1A0;
		const uintptr_t JobName = 0x90;
	}

	namespace Userdata
	{
		const uintptr_t Identity = 0x30;
		const uintptr_t Capabilities = 0x48;
	}

	namespace DataModel
	{
		namespace ScriptContext
		{
			const uintptr_t LuaState = 0x1C8;
		}
	}
}

namespace RBX
{
	using _Print = int(__fastcall*)(int, const char*, ...);
	inline auto Print = (_Print)Update::Print;

	using _GetTaskScheduler = uintptr_t(__fastcall*)();
	inline auto GetTaskScheduler = (_GetTaskScheduler)Update::GetTaskScheduler;

	using _Luau_Execute = void(__fastcall*)(lua_State*);
	inline auto Luau_Execute = (_Luau_Execute)Update::Luau_Execute;

	using _PushInstance = int64_t(__fastcall*)(lua_State*, void*);
	inline auto PushInstance = (_PushInstance)Update::PushInstance;

	using taskdefer_rbx = uintptr_t(__cdecl*)(lua_State* rl);
	inline auto Task_Defer = (taskdefer_rbx)Update::TaskDefer;

	using luavmload_rbx = uintptr_t(__fastcall*)(lua_State* rl, std::string* source, const char* chunk, int env);
	inline auto LuaVm_Load = (luavmload_rbx)Update::LuaVMLoad;

}