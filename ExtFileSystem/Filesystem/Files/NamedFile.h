#pragma once

#include "FileMemory.h"
#include "../../Util/logging.h"
#include "../ProxyFileSystem.h"

class NamedFile : public FileMemory
{
public:
	NamedFile(FileMemory *wrapped, wchar_t const *filename, ExtArchiveData *ext_archive_data);

	//FileMemory *dispose(bool disposing) override;
	~NamedFile() override;
	bool close_file() override;
	bool seek(uint64_t dist, bool absolute) override;
	uint64_t read(void *dest, uint64_t length) override;
	uint64_t read_from(void *buffer, uint64_t pos, uint64_t length) override;
	bool is_open() override;
	void *get_data_ptr() override;
	uint64_t tell() override;
	uint64_t length() override;
	bool set_file(void *data, uint64_t data_length, uint64_t file_offset) override;
	bool set_file2(void *data, uint64_t data_length, uint64_t file_offset) override;
	size_t move_memory(void *dest, void *src, size_t len) override;

	std::wstring filename;
	FileMemory *wrapped;
	ExtArchiveData *ext_archive_data;

	DEF_LOGGER;
};
