#include "NamedFile.h"

NamedFile::NamedFile(FileMemory *wrapped, wchar_t *filename)
{
	this->wrapped = wrapped;
	this->filename = filename;
}

FileMemory * NamedFile::dispose(bool disposing)
{
	wrapped->dispose(disposing);
	if(disposing)
		delete this;
	return this;
}

bool NamedFile::close_file()
{
	return wrapped->close_file();
}

bool NamedFile::seek(uint64_t dist, bool absolute)
{
	return wrapped->seek(dist, absolute);
}

uint64_t NamedFile::read(void *dest, uint64_t length)
{
	return wrapped->read(dest, length);
}

uint64_t NamedFile::read_from(void *buffer, uint64_t pos, uint64_t length)
{
	return wrapped->read_from(buffer, pos, length);
}

bool NamedFile::is_open()
{
	return wrapped->is_open();
}

void *NamedFile::get_data_ptr()
{
	return wrapped->get_data_ptr();
}

uint64_t NamedFile::tell()
{
	return wrapped->tell();
}

uint64_t NamedFile::length()
{
	return wrapped->length();
}

bool NamedFile::set_file(void *data, uint64_t data_length, uint64_t file_offset)
{
	return wrapped->set_file(data, data_length, file_offset);
}

bool NamedFile::set_file2(void *data, uint64_t data_length, uint64_t file_offset)
{
	return wrapped->set_file2(data, data_length, file_offset);
}

size_t NamedFile::move_memory(void *dest, void *src, size_t len)
{
	return wrapped->move_memory(dest, src, len);
}
