#include  "CSVAppendParser.h"

CSVAppendParser::CSVAppendParser(ICsvParser *original_parser) : original_parser(original_parser), cols(0), rows(0),
                                                                values(nullptr)
{
}

void CSVAppendParser::clear_append_data()
{
	dispose(true);
}

void CSVAppendParser::initialize_csv(std::vector<std::wstring> const &values)
{
	clear_append_data();
}

ICsvParser *CSVAppendParser::dispose(bool disposing)
{
	delete[] values;
	values = nullptr;

	original_parser->dispose(disposing);

	if (disposing)
		delete this;
	return this;
}

void CSVAppendParser::get_as_bytes(int col, int row, void *dest, int size)
{
	original_parser->get_as_bytes(col, row, dest, size);
}

int CSVAppendParser::copy_str(int col, int row, std::string *str)
{
	return original_parser->copy_str(col, row, str);
}

void CSVAppendParser::get_as_string(int col, int row, void *dest, int size)
{
	original_parser->get_as_string(col, row, dest, size);
}

int CSVAppendParser::get_as_int(int col, int row)
{
	return original_parser->get_as_int(col, row);
}

float CSVAppendParser::get_as_float(int col, int row)
{
	return original_parser->get_as_float(col, row);
}

int CSVAppendParser::get_cell_strlen(int col, int row)
{
	return original_parser->get_cell_strlen(col, row);
}

int CSVAppendParser::get_cell_byte_length(int col, int row)
{
	return original_parser->get_cell_byte_length(col, row);
}

bool CSVAppendParser::is_valid_cell(int col, int row)
{
	return original_parser->is_valid_cell(col, row);
}

bool CSVAppendParser::is_valid()
{
	return original_parser->is_valid();
}

int CSVAppendParser::get_cols()
{
	return original_parser->get_cols();
}

int CSVAppendParser::get_rows()
{
	return original_parser->get_rows();
}

int CSVAppendParser::get_as_bool(int col, int row)
{
	return original_parser->get_as_bool(col, row);
}
