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

	bool directory_exists(const std::string&);

	bool is_pdf(const std::string&);

	void build_file_array(const std::string&, std::vector<fs::directory_entry>&);

	void rename_file(const fs::directory_entry&, const int, const int);

	void rename_file(const fs::directory_entry&, const int, const int, const int);

	void rename_temp_filename(std::string&, const int, const int);

	void rename_temp_filename(std::string&, const int, const int, const int);
}