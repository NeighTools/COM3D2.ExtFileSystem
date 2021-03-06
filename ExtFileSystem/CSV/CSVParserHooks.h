#pragma once

#include "ICsvParser.h"
#include "../Filesystem/Files/FileMemory.h"
#include "../Util/hook.h"
#include "../Filesystem/Files/NamedFile.h"
#include "CSVAppendParser.h"
#include "../Util/logging.h"

struct CSVParserData
{
	ICsvParser *csv_parser;
	void *data;
	size_t data_length;
};

DEF_HOOK(bool, DLL_CSV_Open, CSVParserData* parser_data, FileMemory* file)
{
	LOG("Calling DLL_CSV_Open!");

	auto named_file = dynamic_cast<NamedFile*>(file);
	if (!named_file)
		return DLL_CSV_Open_original(parser_data, file);

	auto append_parser = dynamic_cast<CSVAppendParser*>(parser_data->csv_parser);
	auto &csv_append_paths = named_file->ext_archive_data->csv_append_paths;

	auto append_paths = csv_append_paths.find(named_file->filename);

	LOG("Got " << csv_append_paths.size() << " items in CSV append paths");

	if (append_paths == csv_append_paths.end())
	{
		LOG("Nothing to append! Returning back...");

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

	LOG("Found stuff to append! Calling parser normally...");

	auto orig = DLL_CSV_Open_original(parser_data, file);


	if (!append_parser)
	{
		LOG("No append parser initialized! Creating one...");
		append_parser = new CSVAppendParser(parser_data->csv_parser);
		ATTACH_LOGGER(append_parser, LogStream);
	}

	LOG("No append parser initialized! Initializing parser...");
	append_parser->initialize_csv(append_paths->second);

	parser_data->csv_parser = append_parser;

	return orig;
}
