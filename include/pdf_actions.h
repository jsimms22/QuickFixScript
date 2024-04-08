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
	
	// Determines what month of the year should be used for publication date
	std::string determine_date(const int);

	// Retrieves the filename and path for either the .pdf or .html version of the title page
	std::string get_dir(const std::string&);

	// Retrieves the filename and path for either the .pdf or .html version of the title page
	std::string get_path(const std::string&, const pdf::FileType);

	// Updates the top level volume and issue number for the title page
	std::string update_title(const std::string&, const int, const int);

	// Define regex for edge cases that occassional occur in the citation and are not handled by update_title()
	std::string update_citation(const std::string&, const int, const int, const std::array<std::string,2>&);

	// Updates the stand-alone title page in the html format
	// This does NOT update the publication paper itself
	void update_html(const std::string&, const int, const int, const std::array<std::string,2>&);

	// Updates the stand-alone title page in the pdf format by converting the updated html title
	// This does NOT update the publication paper itself
	void update_pdf(const std::string&);

	// Retrieve the full path of the targeted paper
	// If the file is moved before the script is run, this will return return an empty string
	std::string get_pub_paper_path(const fs::directory_entry, const std::string&, const std::string);

	// Uses a title offset to determine where the actual paper begins and removes any pages before this number
	void remove_title_page(const fs::directory_entry, const std::string&, const std::string, const int);

	// Concatenates a stand-alone title page .pdf with the paper .pdf
	void update_title_page(const fs::directory_entry, const std::string&, const std::string);
}