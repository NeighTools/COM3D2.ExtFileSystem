#pragma once

#include "ICsvParser.h"
#include "FileMemory.h"
#include "hook.h"
#include "NamedFile.h"
#include "ProxyFileSystem.h"
#include "CSVAppendParser.h"

struct CSVParserData
{
	ICsvParser *csv_parser;
	void *data;
	size_t data_length;
};

DEF_HOOK(bool, DLL_CSV_Open, CSVParserData* parser_data, FileMemory* file)
{
	auto named_file = dynamic_cast<NamedFile*>(file);
	if (!named_file)
		return DLL_CSV_Open_original(parser_data, file);

	auto append_parser = dynamic_cast<CSVAppendParser*>(parser_data->csv_parser);
	auto append_paths = csv_append_paths.find(named_file->filename);

	if (append_paths == csv_append_paths.end())
	{
		if (append_parser)
		{
			append_parser->clear_append_data();
			parser_data->csv_parser = append_parser->original_parser;
		}
		auto orig = DLL_CSV_Open_original(parser_data, file);
		if (append_parser)
			parser_data->csv_parser = append_parser;

		return orig;
	}

	auto orig = DLL_CSV_Open_original(parser_data, file);

	if (!append_parser)
		append_parser = new CSVAppendParser(parser_data->csv_parser);
	append_parser->initialize_csv(append_paths->second);

	parser_data->csv_parser = append_parser;

	return orig;
}
