#include  "CSVAppendParser.h"
#include <fstream>
#include <sstream>
#include "logging.h"

void split(std::vector<std::string> &result, std::string &str, char separator)
{
	std::stringstream buffer;
	size_t quote_level = 0;

	for (auto &c : str)
	{
		// Paired quotes + separator => single value
		if (c == separator && quote_level % 2 == 0)
		{
			result.emplace_back(buffer.str());
			buffer.clear();
			quote_level = 0;
		}
			// Quote
		else if (c == '"')
		{
			if (quote_level % 2 == 1)
				buffer << '"';
			quote_level++;
		}
			// Non-quote char
		else
		{
			if (std::isspace(c) && quote_level == 0)
				continue;

			if (quote_level % 2 == 0)
			{
				// Invalid line, return
				result.clear();
				return;
			}

			buffer << c;
		}
	}
}

CSVAppendParser::CSVAppendParser(ICsvParser *original_parser) : original_parser(original_parser), cols(0), rows(0)
{
	LOG("[CSV] ctor");
}

constexpr bool CSVAppendParser::is_valid_append_cell(int col, int row) const
{
	return col < cols && (row - original_rows) < rows;
}

constexpr int CSVAppendParser::get_cell_index(int col, int row) const
{
	return col + (row - original_rows) * cols;
}

void CSVAppendParser::clear_append_data()
{
	LOG("[CSV] Clearing append data");
	cells.clear();
}

void CSVAppendParser::initialize_csv(std::vector<std::wstring> const &paths)
{
	LOG("[CSV] Initializing append data with " << paths.size() << " append CSVs");

	clear_append_data();

	std::vector<std::string> lines;
	std::vector<std::vector<std::string>> csv_file;

	for (auto &path : paths)
	{
		std::ifstream file(path);

		while (!file.eof())
		{
			auto &line = lines.emplace_back();
			std::getline(file, line);

			auto &cells = csv_file.emplace_back();
			split(cells, line, ',');

			if (cells.empty())
				csv_file.pop_back();
		}
	}

	cols = original_parser->get_cols();
	original_rows = original_parser->get_rows();
	rows = csv_file.size();

	cells.resize(cols * rows);

	for (int row = 0; row < rows; ++row)
	{
		auto &row_entries = csv_file[row];
		for (int col = 0; col < cols; ++col)
		{
			if (col < row_entries.size())
				cells[col + row * cols] = row_entries[col];
		}
	}
}

ICsvParser *CSVAppendParser::dispose(bool disposing)
{
	LOG("[CSV] Disposing");

	clear_append_data();

	original_parser->dispose(disposing);
	delete original_parser;

	if (disposing)
		delete this;
	return this;
}

void CSVAppendParser::get_as_bytes(int col, int row, void *dest, int size)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") as bytes");
	
	if (original_parser->is_valid_cell(col, row))
	{
		original_parser->get_as_bytes(col, row, dest, size);
		return;
	}

	if (!is_valid_append_cell(col, row))
		return;

	auto &value = cells[get_cell_index(col, row)];

	if (value.empty())
		return;

	memcpy(dest, value.c_str(), min(size, value.size()));
}

int CSVAppendParser::copy_str(int col, int row, std::string *str)
{
	LOG("[CSV] Copying (" << col << ", " << row << ") to a string");

	if (original_parser->is_valid_cell(col, row))
		return original_parser->copy_str(col, row, str);

	if (!is_valid_append_cell(col, row))
		return 0;

	auto &cell = cells[get_cell_index(col, row)];

	if (cell.empty())
		return 0;

	str->assign(cell);
	return cell.size() + 1;
}

void CSVAppendParser::get_as_string(int col, int row, void *dest, int size)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") as a string");

	if (original_parser->is_valid_cell(col, row))
	{
		original_parser->get_as_string(col, row, dest, size);
		return;
	}

	if (!is_valid_append_cell(col, row))
		return;

	auto &value = cells[get_cell_index(col, row)];

	if (value.empty())
		return;

	auto write_size = min(size, value.size() + 1);

	memcpy(dest, value.c_str(), write_size);
}

int CSVAppendParser::get_as_int(int col, int row)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") as int");

	if (original_parser->is_valid_cell(col, row))
	{
		return original_parser->get_as_int(col, row);
	}

	if (!is_valid_append_cell(col, row))
		return 0;

	auto &cell = cells[get_cell_index(col, row)];
	return std::stoi(cell);
}

float CSVAppendParser::get_as_float(int col, int row)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") as float");

	if (original_parser->is_valid_cell(col, row))
	{
		return original_parser->get_as_float(col, row);
	}

	if (!is_valid_append_cell(col, row))
		return 0.0f;

	auto &cell = cells[get_cell_index(col, row)];

	if (cell.empty())
		return 0.0f;

	return std::stof(cell);
}

int CSVAppendParser::get_cell_strlen(int col, int row)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") strlen");

	if (original_parser->is_valid_cell(col, row))
	{
		return original_parser->get_cell_strlen(col, row);
	}

	if (!is_valid_append_cell(col, row))
		return 0;

	return cells[get_cell_index(col, row)].size();
}

int CSVAppendParser::get_cell_byte_length(int col, int row)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") byte length");

	if (original_parser->is_valid_cell(col, row))
	{
		return original_parser->get_cell_byte_length(col, row);
	}

	if (!is_valid_append_cell(col, row))
		return 0;

	auto result = cells[get_cell_index(col, row)].size();
	return result == 0 ? 0 : result + 1;
}

bool CSVAppendParser::is_valid_cell(int col, int row)
{
	return original_parser->is_valid_cell(col, row) || (is_valid_append_cell(col, row) && !cells[
		get_cell_index(col, row)].empty());
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
	return original_parser->get_rows() + rows;
}

bool CSVAppendParser::get_as_bool(int col, int row)
{
	LOG("[CSV] Getting (" << col << ", " << row << ") as boolean");

	if (original_parser->is_valid_cell(col, row))
	{
		return original_parser->get_as_bool(col, row);
	}

	if (!is_valid_append_cell(col, row))
		return false;

	auto &cell = cells[get_cell_index(col, row)];
	if (cell.empty())
		return false;

	return std::stoi(cell) != 0;
}
