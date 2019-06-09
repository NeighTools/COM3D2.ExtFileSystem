#include <string>
#include "logging.h"
#include <optional>
#include "ProxyFileSystem.h"
#include "WindowsFile.h"
#include "NamedFile.h"
#include <functional>

void AddAutoPathForAllFolder(FileSystemArchiveNative *fs)
{
	auto self = GET_EXT_THIS(fs, 0);

	REINIT_LOG(self->log_stream);

	LOG_SELF(self, "Add auto path for all folder!");

	for (auto &p : fs::recursive_directory_iterator(self->proxy_path))
	{
		// If it's a CSV file, map it to the appropriate nei file as a possible append 
		if (p.path().extension() == ".csv")
		{
			LOG_SELF(self, "Got CSV file that is potentially a NEI append");
			auto [kv_pair, ok] = self->csv_append_paths.emplace(p.path().stem().wstring().append(L".nei"),
			                                                    std::vector<std::wstring>());
			LOG_SELF(self, "Mapping " << N(kv_pair->first) << " => " << p.path());
			kv_pair->second.emplace_back(p.path());
		}

		self->name_to_full_map.emplace(p.path().filename(), p.path());
	}

	self->original_functions.AddAutoPathForAllFolder(fs);
}

void AddAutoPath(FileSystemArchiveNative *fs, char *path)
{
	auto self = GET_EXT_THIS(fs, 0);

	LOG_SELF(self, "Adding auto path to " << path);
	self->original_functions.AddAutoPath(fs, path);
}

bool FileExists(FileSystemArchiveNative *fs, char *path)
{
	auto self = GET_EXT_THIS(fs, 4);

	LOG_SELF(self, "Checking if file " << path << " exists");

	if (self->name_to_full_map.find(widen(path)) != self->name_to_full_map.end())
		return true;

	if (exists(self->proxy_path / path))
		return true;

	return self->original_functions.FileExists(fs, path);
}

bool FileExistsWide(FileSystemArchiveNative *fs, wchar_t *path)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "Checking if file " << narrow(path) << " exists");

	if (self->name_to_full_map.find(path) != self->name_to_full_map.end())
		return true;

	if (exists(self->proxy_path / path))
		return true;

	return original_functions.FileExistsWide(fs, path);
}

std::vector<std::wstring> *CreateListWide(FileSystemArchiveNative *fs, std::vector<std::wstring> *vec, wchar_t *path,
                                          ListType list_type)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "Creating wide list for path " << narrow(path));

	auto result = self->original_functions.CreateListWide(fs, vec, path, list_type);

	LOG_SELF(self, "Number of files: " << std::dec << result->size());

	for (auto &s : *result)
	{
		LOG_SELF(self, "Got file: " << narrow(s));
	}

	auto search_path = self->proxy_path / path;

	if (!exists(search_path))
		return result;

	if (list_type == TopFile || list_type == TopFolder)
	{
		LOG_SELF(self, "Searching for top contents at " << narrow(search_path));

		for (auto &p : fs::directory_iterator(search_path,
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

		for (auto &p : fs::recursive_directory_iterator(search_path,
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

std::vector<std::string> *CreateList(FileSystemArchiveNative *fs, std::vector<std::string> *list, char *file_path,
                                     ListType list_type)
{
	auto self = GET_EXT_THIS(fs, 4);
	LOG_SELF(self, "Creating normal list for path " << file_path);
	auto result = self->original_functions.CreateList(fs, list, file_path, list_type);
	return result;
}

FileMemory *get_file(ExtArchiveData *ext, wchar_t const *file_path, std::function<void*()> const &original)
{
	FileMemory *result = nullptr;

	auto res = ext->name_to_full_map.find(file_path);
	auto path = ext->proxy_path / file_path;
	if (res != ext->name_to_full_map.end())
	{
		LOG_SELF(ext, "FILE: Loading from " << narrow(res->second));

		auto win_logger = new WindowsFile(res->second);
		ATTACH_LOGGER(win_logger, ext->log_stream);
		result = win_logger;
	}
	else if (exists(path))
	{
		LOG_SELF(ext, "FILE: Loading from " << narrow(path));

		auto win_logger = new WindowsFile(path);
		ATTACH_LOGGER(win_logger, ext->log_stream);
		result = win_logger;
	}
	result = reinterpret_cast<FileMemory*>(original());

	// Handle CSV append by creating a named file
	if (path.extension() == ".nei")
		result = new NamedFile(result, file_path, ext);

	return result;
}

void *GetFile(FileSystemArchiveNative *fs, char *file_str)
{
	auto self = GET_EXT_THIS(fs, 4);

	LOG_SELF(self, "FILE: " << file_str);

	auto wide_path = widen(file_str);
	return get_file(self, wide_path.c_str(), [&self, &file_str, &fs]()
	{
		return self->original_functions.GetFile(fs, file_str);
	});
}

void *GetFileWide(FileSystemArchiveNative *fs, wchar_t *path)
{
	auto self = GET_EXT_THIS(fs, 2);

	LOG_SELF(self, "FILE (WIDE): " << narrow(path));

	return get_file(self, path, [&self, &path, &fs]()
	{
		return self->original_functions.GetFileWide(fs, path);
	});
}
