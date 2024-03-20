#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <array>
#include <cstdlib>

namespace rdf
{
	namespace fs = std::filesystem;

	std::string get_acronym(const std::string&, char);

	std::string find_rdf_dir(const std::string&);

	std::string get_rdf_path(const std::string&);

	void update_rdf_line(const std::string&, const std::string, const std::string);
}