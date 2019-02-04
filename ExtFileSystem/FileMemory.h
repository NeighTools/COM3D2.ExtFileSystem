#pragma once

#include <cstdint>

/**
 * A native file pointer of cm3d2.dll
 * 
 * Used by FileSystemArchive to open files from ARCs
 */
class FileMemory // NOLINT(cppcoreguidelines-special-member-functions)
{
protected:
	~FileMemory() = default;
public:
	virtual FileMemory* dispose(bool disposing) = 0;
	virtual bool close_file() = 0;
	virtual bool seek(uint64_t dist, bool absolute) = 0;
	virtual uint64_t read(void* dest, uint64_t length) = 0;
	virtual uint64_t read_from(void* buffer, uint64_t pos, uint64_t length) = 0;
	virtual bool is_open() = 0;
	virtual void* get_data_ptr() = 0;
	virtual uint64_t tell() = 0;
	virtual uint64_t length() = 0;
	virtual bool set_file(void* data, uint64_t data_length, uint64_t file_offset) = 0;
	virtual bool set_file2(void* data, uint64_t data_length, uint64_t file_offset) = 0;
	virtual size_t move_memory(void* dest, void* src, size_t len) = 0;
};
