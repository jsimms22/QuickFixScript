#pragma once

#include <iostream>
#include <filesystem>
#include <regex>
#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

namespace file
{
	namespace fs = std::filesystem;

	// Verifies the directory exists
	bool directory_exists(const std::string&);

	// Verifiesd the filetype is .pdf
	bool is_pdf(const std::string&);

	// Uses regex to determine proper numerical order and avoids cases where -P9 is ordered after -P10, etc
	bool compare_filenames(const std::filesystem::directory_entry&, const std::filesystem::directory_entry&);

	// Deterministically orders the files in vector by their paper number
	void sort_files(std::vector<std::filesystem::directory_entry>&);

	// Fills a vector with filenames entries for every .pdf in a directory
	void build_file_vec(const std::string&, std::vector<fs::directory_entry>&);

	// Renames the entry's filename in filesystem with the a new year, volume, and issue number
	void rename_file(const fs::directory_entry&, const int, const int);

	// Renames the entry's filename in filesystem with the a new year, volume, issue, and paper number
	void rename_file(const fs::directory_entry&, const int, const int, const int);

	// Renames a temp string for updating fields in a database since the entry class is non-mutable
	void rename_temp_filename(std::string&, const int, const int);

	// Renames a temp string for updating fields in a database since the entry class is non-mutable, overloads to update paper number
	void rename_temp_filename(std::string&, const int, const int, const int);
}