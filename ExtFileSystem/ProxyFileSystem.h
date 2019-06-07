/*
 * ProxyFileSystem.h -- Definition of hooks for the ARC filesystem
 *
 * Basic overview:
 * - Here we keep definitions (and implementations) of the hooked methods
 * - Each method gets a this pointer to the vtable the currently executing method resides in
 * - To obtain the extra data, we thus need to know what vtable each method belongs to (and the size of the original object) 
 */
#pragma once

#include <vector>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

// Size of the filesystem object
#if _M_IX86
#define FS_ARCHIVE_OBJECT_BASE_SIZE 0x12
#elif _M_X64
#define FS_ARCHIVE_OBJECT_BASE_SIZE 0x10
#endif

// A helper macro to access the extended archive data
// Parameters:
// fs -- Pointer to the current filesystem vtable (basically this in C++)
// vtable_index -- index of the ARCArchive vtable the current function is located
#define GET_EXT_THIS(fs, vtable_index) \
	(reinterpret_cast<ExtArchiveData*>(*(fs - vtable_index + FS_ARCHIVE_OBJECT_BASE_SIZE)))

// A helper that creates a function signature and a function pointer type with the same signature
#define DEF_FUNC(name, return_type, ...) \
	return_type name(__VA_ARGS__); \
	typedef return_type (*name##_t)(__VA_ARGS__);

enum ListType
{
	// Only return the folders of the top folder
	TopFolder,
	// Returns all folders
	AllFolder,
	// Returns only the files of the top folder
	TopFile,
	// Returns all files in all folders
	AllFile
};


struct OriginalFunctions;
using FileSystemArchiveNative = void*;

// First vtable

//DEF_FUNC(SetBaseDirectory, void, FileSystemArchiveNative *fs, char *path);
//DEF_FUNC(AddArchive, void, FileSystemArchiveNative *fs, char *path);
DEF_FUNC(AddAutoPathForAllFolder, void, FileSystemArchiveNative *fs);
DEF_FUNC(AddAutoPath, void, FileSystemArchiveNative *fs, char *path);

// Second vtable

DEF_FUNC(GetFileWide, void*, FileSystemArchiveNative *fs, wchar_t *path);
DEF_FUNC(FileExistsWide, bool, FileSystemArchiveNative *fs, wchar_t *path);
DEF_FUNC(CreateListWide, std::vector<std::wstring> *, FileSystemArchiveNative *fs, std::vector<std::wstring> *vec,
	wchar_t *path, ListType list_type);

// Third vtable

DEF_FUNC(CreateList, std::vector<std::string> *, FileSystemArchiveNative *fs, std::vector<std::string> *list, char *path, ListType list_type);
DEF_FUNC(GetFile, void*, FileSystemArchiveNative *fs, char *file_str);
DEF_FUNC(FileExists, bool, FileSystemArchiveNative *fs, char *file_str);

// Pointers to original functions in case we need them
struct OriginalFunctions
{
	//SetBaseDirectory_t SetBaseDirectory;
	//AddArchive_t AddArchive;
	AddAutoPathForAllFolder_t AddAutoPathForAllFolder;
	AddAutoPath_t AddAutoPath;
	CreateList_t CreateList;
	GetFile_t GetFile;
	FileExists_t FileExists;
	GetFileWide_t GetFileWide;
	FileExistsWide_t FileExistsWide;
	CreateListWide_t CreateListWide;
};

// Global data used by all proxied filesystems
static OriginalFunctions original_functions;
static fs::path proxy_path;
static std::unordered_map<std::wstring, std::wstring> name_to_full_map;

// Extra data appended to each archive object
struct ExtArchiveData
{
	FileSystemArchiveNative* base;
	OriginalFunctions& original_functions;
	fs::path& proxy_path;
	size_t proxy_path_length;
	std::unordered_map<std::wstring, std::wstring>& name_to_full_map;
	std::ofstream* log_stream;

	ExtArchiveData(fs::path& proxy_path, OriginalFunctions& original_functions,
	               std::unordered_map<std::wstring, std::wstring>& name_to_full_map)
		: original_functions(original_functions),
		  proxy_path(proxy_path),
		  name_to_full_map(name_to_full_map)
	{
		proxy_path_length = proxy_path.generic_wstring().size();
	}
};
