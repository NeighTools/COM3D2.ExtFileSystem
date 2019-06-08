#pragma once

#include "ICsvParser.h"
#include "FileMemory.h"
#include "hook.h"
#include "NamedFile.h"

struct CSVParserData
{
	ICsvParser *csv_parser;
	void *data;
	size_t data_length;
};

DEF_HOOK(CSVParserData*, DLL_CSV_CreateCsvParser)
{
	return nullptr;
}

DEF_HOOK(bool, DLL_CSV_Open, CSVParserData* parser_data, FileMemory* file)
{
	auto named_file = dynamic_cast<NamedFile*>(file);

	if (!named_file)
		return DLL_CSV_Open_original(parser_data, file);

	

	return true;
}
