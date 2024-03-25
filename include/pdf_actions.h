#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <cstdio>
#include <cstdlib>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

namespace pdf 
{
	enum class FileType {
		PDF,
		HTML
	};

	std::string get_html_dir(const std::string&);

	std::string get_html_path(const std::string&);

	std::string get_path(const std::string&, const pdf::FileType);

	std::string update_title(const std::string&, int, int);

	std::string update_citation(const std::string&, int, int);

	void update_html(const std::string&, int, int);

	void update_pdf(const std::string&);
}