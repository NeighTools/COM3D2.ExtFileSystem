#pragma once

#include <windows.h>

#define DEF_HOOK(ret_type, func, ...) \
	typedef ret_type (*func##_original_t)(__VA_ARGS__);\
	static func##_original_t func##_original = nullptr;\
	ret_type func##_hook(__VA_ARGS__)

#define RVA2PTR(t, base, rva) ((t)(((char*)base) + rva))

inline BOOL patch_cur_iat(HMODULE hmodule, char const *targetModule, void *targetFunction, void *hook)
{
	IMAGE_DOS_HEADER *mz = (PIMAGE_DOS_HEADER)hmodule;

	IMAGE_NT_HEADERS *nt = RVA2PTR(PIMAGE_NT_HEADERS, mz, mz->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR *imports = RVA2PTR(PIMAGE_IMPORT_DESCRIPTOR, mz, nt->OptionalHeader.DataDirectory[
		                                           IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (int i = 0; imports[i].Characteristics; i++)
	{
		char *name = RVA2PTR(char*, mz, imports[i].Name);

		if (lstrcmpiA(name, targetModule) != 0)
			continue;

		void **cur = RVA2PTR(void**, mz, imports[i].FirstThunk);

		for (; cur; cur++)
		{
			if (*cur != targetFunction)
				continue;

			MEMORY_BASIC_INFORMATION vmi;

			VirtualQuery(cur, &vmi, sizeof(MEMORY_BASIC_INFORMATION));
			DWORD oldProtection;
			if (!VirtualProtect(vmi.BaseAddress, vmi.RegionSize, PAGE_READWRITE, &oldProtection))
				return FALSE;

			*cur = hook;

			VirtualProtect(vmi.BaseAddress, vmi.RegionSize, oldProtection, &oldProtection);
			return TRUE;
		}
	}

	return FALSE;
}
