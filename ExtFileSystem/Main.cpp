/*
 * Main.cpp -- Main code of the proxy
 * 
 * * Basic overview:
 * - ARCArchive object is a C++ object that inherits from multiple C++ classes/interfaces
 * - C++ objects are basically structures that contain pointers to arrays of function addresses (i.e. vtables)
 * - Each vtable contains pointers to the implementations of each C++ class/interface function
 * - We can thus hook C++ functions by making copies of the vtables and replacing the function pointers
 * - Each C++ object keeps its own pointers to vtables, so we can control which objects get hooked and which do not
 * - We also can append extra data to the object by making a copy of it
 */

#include <windows.h>
#include "Util/logging.h"
#include "Util/hook.h"
#include "Util/heap.h"

#include "Filesystem/FileSystemHooks.h"
#include "CSV/CSVParserHooks.h"

#define DllExport __declspec(dllexport)

#define xstr(s) str(s)
#define str(s) #s
#define REDIRECT(func) \
	{ \
		if(lstrcmpiA(name, str(func)) == 0) \
		{\
			func##_original = reinterpret_cast<func##_original_t>(GetProcAddress(hmodule, name));\
			return &func##_hook;\
		}\
	}

void * WINAPI hookGetProcAddress(HMODULE hmodule, char const *name)
{
	REDIRECT(DLL_FileSystem_CreateFileSystemArchive);
	REDIRECT(DLL_CSV_Open);

	return GetProcAddress(hmodule, name);
}

/*
 * Installs the proxy
 */
extern "C" void DllExport Install(wchar_t *modsPath)
{
	INIT_LOG(".");
	LOG("Filesystem proxy started!");

	initialize_heap();

	proxy_path = modsPath;

	if (!is_directory(proxy_path))
	{
		LOG("Path " << narrow(proxy_path) << " is not a directory! Creating one!");
		return;
	}

	if (!patch_cur_iat(GetModuleHandleA("mono"), "kernel32.dll", &GetProcAddress, &hookGetProcAddress))
	{
		LOG("Failed to enable hook!");
	}

	LOG("Hook done!");
}
