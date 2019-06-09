#pragma once
#include <string>

/**
 * A native CSV parser
 */
class ICsvParser // NOLINT(hicpp-special-member-functions, cppcoreguidelines-special-member-functions)
{
public:
	virtual ~ICsvParser() = default;
	virtual void get_as_bytes(int col, int row, void *dest, int size) = 0;
	virtual int copy_str(int col, int row, std::string *str) = 0;
	virtual void get_as_string(int col, int row, void *dest, int size) = 0;
	virtual int get_as_int(int col, int row) = 0;
	virtual float get_as_float(int col, int row) = 0;
	virtual int get_cell_strlen(int col, int row) = 0;
	virtual int get_cell_byte_length(int col, int row) = 0;
	virtual bool is_valid_cell(int col, int row) = 0;
	virtual bool is_valid() = 0;
	virtual int get_cols() = 0;
	virtual int get_rows() = 0;
	virtual bool get_as_bool(int col, int row) = 0;
};
