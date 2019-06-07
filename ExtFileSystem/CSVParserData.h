#pragma once

#include "ICsvParser.h"

struct CSVParserData
{
	ICsvParser *csv_parser;
	void *data;
	size_t data_length;
};
