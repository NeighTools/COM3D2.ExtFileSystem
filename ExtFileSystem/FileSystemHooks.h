#pragma once
#include "ProxyFileSystem.h"
#include "logging.h"
#include "hook.h"

struct FSArchive
{
	void *fs_object;
	int fs_type;
};

static bool v_table_initialized = false;
static void **FSArchiveVTable = nullptr;
static void **FSArchiveIOVTable = nullptr;
static void **FSArchiveWideFsTable = nullptr;
static std::vector<ExtArchiveData*> extraArchiveDatas;

inline void init_virutal_tables(void **fs_object)
{
#define CPY_VTABLE(dest, index, size) \
	auto dest##_temp = reinterpret_cast<void**>(fs_object[index]);\
	dest = new void*[size]; \
	memcpy(dest, dest##_temp, size * sizeof(void*));

#define SET(dest, index, func) \
	original_functions.func = reinterpret_cast<func##_t>(dest##_temp[index]); \
	dest[index] = &func;

	CPY_VTABLE(FSArchiveVTable, 0, 15);

	//SET(FSArchiveVTable, 5, SetBaseDirectory);
	//SET(FSArchiveVTable, 9, AddArchive);
	SET(FSArchiveVTable, 10, AddAutoPathForAllFolder);
	SET(FSArchiveVTable, 13, AddAutoPath);

	CPY_VTABLE(FSArchiveIOVTable, 4, 3);

	SET(FSArchiveIOVTable, 0, GetFile);
	SET(FSArchiveIOVTable, 1, FileExists);
	SET(FSArchiveIOVTable, 2, CreateList);

	CPY_VTABLE(FSArchiveWideFsTable, 2, 3);

	SET(FSArchiveWideFsTable, 0, GetFileWide);
	SET(FSArchiveWideFsTable, 1, FileExistsWide);
	SET(FSArchiveWideFsTable, 2, CreateListWide);

#undef CPY_VTABLE
#undef SET
}

DEF_HOOK(void, DLL_FileSystem_CreateFileSystemArchive, FSArchive* dest)
{
	LOG("Creating file system. Struct address: " << std::hex << dest);
	DLL_FileSystem_CreateFileSystemArchive_original(dest);

	if (dest->fs_object == nullptr)
		return;

	auto fs_obj = reinterpret_cast<void**>(dest->fs_object);

	LOG("Filesystem object at: " << std::hex << fs_obj);

	if (!v_table_initialized)
	{
		LOG("Loading virtual tables!");
		init_virutal_tables(fs_obj);
		v_table_initialized = true;
		LOG("Virtual tables loaded and copied over!");
	}

	// Create a carbon copy of the archive object
	auto new_obj = new void* [FS_ARCHIVE_OBJECT_BASE_SIZE + 1];
	memcpy(new_obj, fs_obj, FS_ARCHIVE_OBJECT_BASE_SIZE * sizeof(void*));

	// Redirect the vtables
	new_obj[0] = FSArchiveVTable;
	new_obj[2] = FSArchiveWideFsTable;
	new_obj[4] = FSArchiveIOVTable;

	// Create external data to store pointers to global data
	auto ext_data = new ExtArchiveData(proxy_path, original_functions, name_to_full_map, csv_append_paths);
	ext_data->base = new_obj;
	ext_data->log_stream = LogStream;

	extraArchiveDatas.push_back(ext_data);

	// Append extra data to the new object
	new_obj[FS_ARCHIVE_OBJECT_BASE_SIZE] = ext_data;

	LOG("Freeing up original archive object!");
	HeapFree(heap, 0, fs_obj);
	LOG("Object freed!");

	// Return the hooked object
	dest->fs_object = new_obj;
}
