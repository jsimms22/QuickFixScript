#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <cstdio>
#include <cstdlib>
//#include <poppler/cpp/poppler-document.h>
//#include <poppler/cpp/poppler-page.h>
#include <filesystem>
#include <array>

namespace pdf 
{
	namespace fs = std::filesystem;

	enum class FileType {
		PDF,
		HTML
	};

	std::string get_dir(const std::string&);

	std::string get_path(const std::string&, const pdf::FileType);

	std::string update_title(const std::string&, const int, const int);

	std::string update_citation(const std::string&, const int, const int, const std::array<std::string,2>&);

	void update_html(const std::string&, const int, const int, const std::array<std::string,2>&);

	void update_pdf(const std::string&);

	std::string get_pub_paper_path(const fs::directory_entry, const std::string&, const std::string);

	void remove_title_page(const fs::directory_entry, const std::string&, const std::string);

	void update_title_page(const fs::directory_entry, const std::string&, const std::string);
}