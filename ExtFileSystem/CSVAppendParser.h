#pragma once

#include "ICsvParser.h"
#include <vector>

class CSVAppendParser : public ICsvParser
{
public:
	CSVAppendParser(ICsvParser *original_parser);

	void clear_append_data();
	void initialize_csv(std::vector<std::wstring> const &values);
	ICsvParser *dispose(bool disposing) override;
	void get_as_bytes(int col, int row, void *dest, int size) override;
	int copy_str(int col, int row, std::string *str) override;
	void get_as_string(int col, int row, void *dest, int size) override;
	int get_as_int(int col, int row) override;
	float get_as_float(int col, int row) override;
	int get_cell_strlen(int col, int row) override;
	int get_cell_byte_length(int col, int row) override;
	bool is_valid_cell(int col, int row) override;
	bool is_valid() override;
	int get_cols() override;
	int get_rows() override;
	int get_as_bool(int col, int row) override;

	ICsvParser* original_parser;

private:
	int cols, rows;
	char *values;
};
