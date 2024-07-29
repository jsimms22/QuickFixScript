#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <cstdlib>
#include <cassert>

namespace rdf
{
	namespace fs = std::filesystem;

	// Extracts the publication's acronym from a paper's id
	std::string get_acronym(const std::string&, const char);

	// Determines the high level directory based on the id's acronym
	// Only supports: EB, EBFT08, VUECON, and 777WPS
	std::string find_rdf_dir(const std::string&);

	// Determines the full path of an .rdf for the given paper's id
	std::string get_rdf_path(const std::string&);

	// Read from the current state of an rdf into a temp file 
	// while updating lines containing the criteria
	void write_to_temp(const std::string&, const std::string&, 
					   const std::string, const std::string);

	// Read from a temp file and overwrite the current state of an rdf file
	void read_from_temp(const std::string&, const std::string&);

	// Updates a line in an rdf where the line contains a search criteria
	void update_rdf_line(const std::string&, const std::string, const std::string);
}