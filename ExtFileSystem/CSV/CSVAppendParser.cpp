#include "CSVAppendParser.h"
#include <fstream>
#include <sstream>
#include "../Util/logging.h"

void split(std::vector<std::string> &result, std::string &str, char separator)
{
	std::stringstream buffer, whitespace_buffer;
	size_t quote_level = 0;
	bool is_quoted = false;
	size_t read_count = 0;
	size_t whitespace_count = 0;

	for (auto &c : str)
	{
		// Buffer in whitespace
		const auto is_whitespace = std::isspace(c);
		const auto should_separate = c == separator && (!is_quoted || (is_quoted && quote_level % 2 == 0));

		if (is_whitespace)
		{
			whitespace_buffer << c;
			whitespace_count++;
			continue;
		}

		if (!is_whitespace && whitespace_count > 0)
		{
			// If the char is not whitespace but we are reading value, append the space
			// Otherwise (not inside value) we'll trim it
			if (read_count > 0 && !should_separate)
			{
				buffer << whitespace_buffer.str();
				read_count += whitespace_count;
			}
			whitespace_count = 0;
			whitespace_buffer.str(std::string());
			whitespace_buffer.clear();
		}

		// Paired quotes + separator => single value
		if (should_separate)
		{
			auto &result_str = result.emplace_back(buffer.str());
			result_str.append(whitespace_buffer.str());

			buffer.str(std::string());
			buffer.clear();
			quote_level = 0;
			is_quoted = false;
			read_count = 0;
			whitespace_count = 0;
			whitespace_buffer.str(std::string());
			whitespace_buffer.clear();
		}
			// Quote
		else if (c == '"')
		{
			// Non-quote mode
			if (read_count == 0 && quote_level == 0)
			{
				is_quoted = true;
				quote_level++;
				if (whitespace_count > 0)
				{
					whitespace_buffer.str(std::string());
					whitespace_buffer.clear();
					whitespace_count = 0;
				}
				continue;
			}
			if (is_quoted)
				quote_level++;

			if (!is_quoted || quote_level % 2 == 1)
			{
				buffer << c;
				read_count++;
			}
		}
			// Non-quote char
		else
		{
			if (is_quoted && quote_level != 0 && quote_level % 2 == 0)
			{
				// Invalid line, return
				result.clear();
				return;
			}

			buffer << c;
			read_count++;
		}
	}

	// Add the last contents in, if there are any
	if (read_count != 0)
		result.emplace_back(buffer.str());
}

CSVAppendParser::CSVAppendParser(ICsvParser *original_parser) : original_parser(original_parser), cols(0), rows(0),
                                                                original_rows(0)
{
}

CSVAppendParser::~CSVAppendParser()
{
	CLASS_LOG("[CSV] Disposing");

	delete original_parser;
}

constexpr bool CSVAppendParser::is_original_cell(int col, int row) const
{
	return col < cols && row < original_rows;
}

constexpr bool CSVAppendParser::is_append_cell(int col, int row) const
{
	return col < cols && (row - original_rows) < rows;
}

constexpr int CSVAppendParser::get_cell_index(int col, int row) const
{
	return col + (row - original_rows) * cols;
}

void CSVAppendParser::clear_append_data()
{
	CLASS_LOG("[CSV] Clearing append data");
	cells.clear();
}

void CSVAppendParser::initialize_csv(std::vector<std::wstring> const &paths)
{
	CLASS_LOG("[CSV] Initializing append data with " << paths.size() << " append CSVs");

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

			CLASS_LOG("[CSV] Got line: " << line);

			auto &cells = csv_file.emplace_back();
			split(cells, line, ';');

			CLASS_LOG("[CSV] Got " << cells.size() << " cells from the previous line");

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
			auto &cell = row_entries[col];

			CLASS_LOG("(" << col << ", " << (row + original_rows) << ") <== " << cell);

			if (col < row_entries.size())
				cells[col + row * cols] = cell;
		}
	}
}

void CSVAppendParser::get_as_bytes(int col, int row, void *dest, int size)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") as bytes");

	if (is_original_cell(col, row))
	{
		original_parser->get_as_bytes(col, row, dest, size);
		return;
	}

	if (!is_append_cell(col, row))
		return;

	auto &value = cells[get_cell_index(col, row)];

	if (value.empty())
		return;

	memcpy(dest, value.c_str(), min(size, value.size()));
}

int CSVAppendParser::copy_str(int col, int row, std::string *str)
{
	CLASS_LOG("[CSV] Copying (" << col << ", " << row << ") to a string");

	if (is_original_cell(col, row))
		return original_parser->copy_str(col, row, str);

	if (!is_append_cell(col, row))
		return 0;

	auto &cell = cells[get_cell_index(col, row)];

	if (cell.empty())
		return 0;

	str->assign(cell);
	return cell.size() + 1;
}

void CSVAppendParser::get_as_string(int col, int row, void *dest, int size)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") as a string");

	if (is_original_cell(col, row))
	{
		original_parser->get_as_string(col, row, dest, size);
		return;
	}

	if (!is_append_cell(col, row))
		return;

	auto &value = cells[get_cell_index(col, row)];

	if (value.empty())
		return;

	auto write_size = min(size, value.size() + 1);

	memcpy(dest, value.c_str(), write_size);
}

int CSVAppendParser::get_as_int(int col, int row)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") as int");

	if (is_original_cell(col, row))
		return original_parser->get_as_int(col, row);

	if (!is_append_cell(col, row))
		return 0;

	auto &cell = cells[get_cell_index(col, row)];
	const auto res = std::stoi(cell);

	CLASS_LOG("[CSV] (" << col << ", " << row << ") = " << res);

	return res;
}

float CSVAppendParser::get_as_float(int col, int row)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") as float");

	if (is_original_cell(col, row))
		return original_parser->get_as_float(col, row);

	if (!is_append_cell(col, row))
		return 0.0f;

	auto &cell = cells[get_cell_index(col, row)];

	if (cell.empty())
		return 0.0f;

	return std::stof(cell);
}

int CSVAppendParser::get_cell_strlen(int col, int row)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") strlen");

	if (is_original_cell(col, row))
		return original_parser->get_cell_strlen(col, row);

	if (!is_append_cell(col, row))
		return 0;

	return cells[get_cell_index(col, row)].size();
}

int CSVAppendParser::get_cell_byte_length(int col, int row)
{
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") byte length");

	if (is_original_cell(col, row))
		return original_parser->get_cell_byte_length(col, row);

	if (!is_append_cell(col, row))
		return 0;

	auto result = cells[get_cell_index(col, row)].size();
	return result == 0 ? 0 : result + 1;
}

bool CSVAppendParser::is_valid_cell(int col, int row)
{
	return original_parser->is_valid_cell(col, row) || (is_append_cell(col, row) && !cells[
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
	CLASS_LOG("[CSV] Getting (" << col << ", " << row << ") as boolean");

	if (is_original_cell(col, row))
		return original_parser->get_as_bool(col, row);

	if (!is_append_cell(col, row))
		return false;

	auto &cell = cells[get_cell_index(col, row)];
	if (cell.empty())
		return false;

	return std::stoi(cell) != 0;
}
