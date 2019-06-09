#include "WindowsFile.h"

WindowsFile::WindowsFile(std::wstring path)
{
	this->stream = std::ifstream(path, std::ios::binary);

	this->stream.seekg(0, std::ios::end);
	this->len = stream.tellg();
	this->stream.seekg(0, std::ios::beg);
}

WindowsFile::~WindowsFile()
{
	CLASS_LOG("Disposing!");
	stream.close();
}

bool WindowsFile::close_file()
{
	CLASS_LOG("Closing file!");
	this->stream.close();
	return false;
}

bool WindowsFile::seek(uint64_t dist, bool absolute)
{
	CLASS_LOG("Seeking data!");
	this->stream.seekg(dist, absolute ? std::ios::beg : std::ios::cur);
	return true;
}

uint64_t WindowsFile::read(void *dest, uint64_t length)
{
	CLASS_LOG("Reading data!");
	this->stream.read((char*)dest, length);
	return this->stream.gcount();
}

uint64_t WindowsFile::read_from(void *buffer, uint64_t pos, uint64_t length)
{
	CLASS_LOG("Reading data from pos!");
	size_t prev = this->stream.tellg();
	this->stream.seekg(pos, std::ios::beg);
	this->stream.read((char*)buffer, length);
	size_t read = stream.gcount();
	this->stream.seekg(prev, std::ios::beg);
	return read;
}

bool WindowsFile::is_open()
{
	CLASS_LOG("Checking if open!");
	return this->stream.is_open();
}

uint64_t WindowsFile::tell()
{
	CLASS_LOG("Telling position");
	return this->stream.tellg();
}

uint64_t WindowsFile::length()
{
	CLASS_LOG("Getting length");
	return this->len;
}

bool WindowsFile::set_file(void *data, uint64_t data_length, uint64_t file_offset)
{
	CLASS_LOG("Setting file");
	return false;
}

bool WindowsFile::set_file2(void *data, uint64_t data_length, uint64_t file_offset)
{
	CLASS_LOG("Setting file2");
	return false;
}

void *WindowsFile::get_data_ptr()
{
	CLASS_LOG("Getting raw data ptr file");
	return nullptr;
}

size_t WindowsFile::move_memory(void *dest, void *src, size_t len)
{
	CLASS_LOG("Moving file ptr");
	return 0;
}
