#include <string>
#include "logging.h"
#include <optional>
#include "ProxyFileSystem.h"
#include "WindowsFile.h"

void AddAutoPathForAllFolder(FileSystemArchiveNative* fs)
{
	auto self = GET_EXT_THIS(fs, 0);

	LOG_SELF(self, "Add auto path for all folder!");

	for (auto& p : fs::recursive_directory_iterator(self->proxy_path))
		self->name_to_full_map[p.path().filename()] = p.path();

	self->original_functions.AddAutoPathForAllFolder(fs);
}

void AddAutoPath(FileSystemArchiveNative* fs, char* path)
{
	auto self = GET_EXT_THIS(fs, 0);

	LOG_SELF(self, "Adding auto path to " << path);
	self->original_functions.AddAutoPath(fs, path);
}

bool FileExists(FileSystemArchiveNative* fs, char* path)
{
	auto self = GET_EXT_THIS(fs, 4);

	LOG_SELF(self, "Checking if file " << path << " exists");

	if (self->name_to_full_map.find(widen(path)) != self->name_to_full_map.end())
		return true;

	if (exists(self->proxy_path / path))
		return true;

	return self->original_functions.FileExists(fs, path);
}

bool FileExistsWide(FileSystemArchiveNative* fs, wchar_t* path)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "Checking if file " << narrow(path) << " exists");

	if (self->name_to_full_map.find(path) != self->name_to_full_map.end())
		return true;

	if (exists(self->proxy_path / path))
		return true;

	return original_functions.FileExistsWide(fs, path);
}

std::vector<std::wstring>* CreateListWide(FileSystemArchiveNative* fs, std::vector<std::wstring>* vec, wchar_t* path,
                                          ListType list_type)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "Creating wide list for path " << narrow(path));

	auto result = self->original_functions.CreateListWide(fs, vec, path, list_type);

	LOG_SELF(self, "Number of files: " << std::dec << result->size());

	for (auto& s : *result)
	LOG_SELF(self, "Got file: " << narrow(s));

	auto search_path = self->proxy_path / path;

	if (!exists(search_path))
		return result;

	if (list_type == TopFile || list_type == TopFolder)
	{
		LOG_SELF(self, "Searching for top contents at " << narrow(search_path));

		for (auto& p : fs::directory_iterator(search_path,
		                                      fs::directory_options::follow_directory_symlink | fs::directory_options::
		                                      skip_permission_denied))
		{
			LOG_SELF(self, "Adding " << narrow(p.path()));
			if (list_type == TopFolder && p.is_regular_file())
				result->emplace_back(p.path().c_str() + self->proxy_path_length);
			else if (list_type == TopFile && p.is_directory())
				result->emplace_back(p.path().c_str() + self->proxy_path_length);
		}
	}
	else
	{
		LOG_SELF(self, "Searching for recursive contents at " << narrow(search_path));

		for (auto& p : fs::recursive_directory_iterator(search_path,
		                                                fs::directory_options::follow_directory_symlink | fs::
		                                                directory_options::skip_permission_denied))
		{
			LOG_SELF(self, "Adding " << narrow(p.path()));

			if (list_type == AllFile && p.is_regular_file())
				result->emplace_back(p.path().c_str() + self->proxy_path_length);
			else if (list_type == AllFolder && p.is_directory())
				result->emplace_back(p.path().c_str() + self->proxy_path_length);
		}
	}

	return result;
}

std::vector<std::string>* CreateList(FileSystemArchiveNative* fs, std::vector<std::string>* list, char* file_path,
                                     ListType list_type)
{
	auto self = GET_EXT_THIS(fs, 4);
	LOG_SELF(self, "Creating normal list for path " << file_path);
	auto result = self->original_functions.CreateList(fs, list, file_path, list_type);
	return result;
}

void* GetFile(FileSystemArchiveNative* fs, char* file_str)
{
	auto self = GET_EXT_THIS(fs, 4);

	LOG_SELF(self, "FILE: " << file_str);

	auto wide_path = widen(file_str);

	auto res = self->name_to_full_map.find(wide_path);
	if (res != self->name_to_full_map.end())
	{
		LOG_SELF(self, "FILE: Loading from " << narrow(res->second));

		auto result = new WindowsFile(res->second);
		ATTACH_LOGGER(result, self->log_stream);
		return result;
	}

	auto path = self->proxy_path / wide_path;
	if (exists(path))
	{
		LOG_SELF(self, "FILE: Loading from " << narrow(path));

		auto result = new WindowsFile(path);
		ATTACH_LOGGER(result, self->log_stream);
		return result;
	}

	return self->original_functions.GetFile(fs, file_str);
}

void* GetFileWide(FileSystemArchiveNative* fs, wchar_t* path)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "FILE (WIDE): " << narrow(path));

	auto res = self->name_to_full_map.find(path);
	if (res != self->name_to_full_map.end())
	{
		LOG_SELF(self, "FILE (WIDE): Loading from " << narrow(res->second));

		auto result = new WindowsFile(res->second);
		ATTACH_LOGGER(result, self->log_stream);
		return result;
	}

	auto file_path = self->proxy_path / path;
	if (exists(file_path))
	{
		LOG_SELF(self, "FILE (WIDE): Loading from " << narrow(file_path));

		auto result = new WindowsFile(file_path);
		ATTACH_LOGGER(result, self->log_stream);
		return result;
	}

	return self->original_functions.GetFileWide(fs, path);
}
