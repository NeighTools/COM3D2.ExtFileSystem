#pragma once

#include "../../Util/logging.h"
#include "FileMemory.h"
#include <fstream>
#include "../ProxyFileSystem.h"

/**
 * \brief File served from the filesystem.
 */
class WindowsFile : public FileMemory // NOLINT
{
public:
	WindowsFile(std::wstring path);

	~WindowsFile() override;
	//WindowsFile* dispose(bool disposing) override;
	bool close_file() override;
	bool seek(uint64_t dist, bool absolute) override;
	uint64_t read(void *dest, uint64_t length) override;
	uint64_t read_from(void *buffer, uint64_t pos, uint64_t length) override;
	bool is_open() override;
	uint64_t tell() override;
	uint64_t length() override;
	bool set_file(void *data, uint64_t data_length, uint64_t file_offset) override;
	bool set_file2(void *data, uint64_t data_length, uint64_t file_offset) override;
	void *get_data_ptr() override;
	size_t move_memory(void *dest, void *src, size_t len) override;
private:
	std::ifstream stream;
	size_t len;

	DEF_LOGGER;
};
