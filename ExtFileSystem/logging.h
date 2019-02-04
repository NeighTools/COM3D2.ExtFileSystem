#pragma once

#include <windows.h>
#include <fstream>
#include <filesystem>

static std::string narrow(std::wstring const& str)
{
	const auto char_len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), nullptr, 0, nullptr, nullptr);

	if (char_len == 0)
		return "";

	std::string result;
	result.resize(char_len);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.size(), const_cast<char*>(result.c_str()), char_len, nullptr,
	                    nullptr);
	return result;
}

static std::string narrow_view(std::wstring_view const& str)
{
	const auto char_len = WideCharToMultiByte(CP_UTF8, 0, str.data(), str.length(), nullptr, 0, nullptr, nullptr);

	if (char_len == 0)
		return "";

	std::string result;
	result.resize(char_len);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), const_cast<char*>(result.c_str()), char_len, nullptr,
	                    nullptr);
	return result;
}

static std::wstring widen(std::string const& str)
{
	const auto char_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), nullptr, 0);

	if (char_len == 0)
		return L"";

	std::wstring result;
	result.resize(char_len);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), const_cast<wchar_t*>(result.c_str()), char_len);
	return result;
}

static std::ofstream* LogStream = nullptr;

#ifdef _VERBOSE

#define DEF_LOGGER public: std::ofstream *logger
#define CLASS_LOG(msg) *this->logger << msg << std::endl
#define ATTACH_LOGGER(obj, log_ptr) obj->logger = log_ptr

#define LOG_SELF(self, msg) *self->log_stream << msg << std::endl
#define LOG(msg) *LogStream << msg << std::endl
#define N(str) narrow(str)
#define INIT_LOG(path) init_log(path)

inline void init_log(std::filesystem::path const& root)
{
	LogStream = new std::ofstream();
	const auto log_file = root / L"extfilesystem.log";
	LogStream->open(log_file, std::ios_base::out | std::ios_base::binary);
}

#else

#define DEF_LOGGER
#define CLASS_LOG
#define ATTACH_LOGGER(obj, log_ptr)

#define LOG_SELF(self, msg)
#define LOG(msg)
#define N(str) ""

#define INIT_LOG(path)

#endif
