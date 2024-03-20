#include "rdf_actions.h"

namespace rdf
{
	std::string get_acronym(const std::string& id, char delimiter)
	{
		std::vector<std::string> pieces;
		std::istringstream iss(id);
		std::string token;

		while (std::getline(iss, token, delimiter)) {
			pieces.push_back(token);
		}

		return pieces[0];
	}

	std::string get_rdf_dir(const std::string& id)
	{
		std::string dir;

		char delimiter = '-';
		std::string pub = rdf::get_acronym(id, delimiter);
		std::cout << "Looking for publication rdf dir (ID:" + id + "): " + pub << std::endl;

		std::array<std::string, 4> acronyms{ "EBFT08","EB","777wps777","VUECON" };

		if (pub.compare(acronyms[0]) == 0) {
			// dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/RePEc/EBF/ebfull";
			// Local testing directory only:
			dir = "C:/Users/work/Desktop/ebfull";
		} else if (pub.compare(acronyms[1]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/RePEc/ebl/ecbull";
		} else if (pub.compare(acronyms[2]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/RePEc/777/777wps";
		} else if (pub.compare(acronyms[3]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/RePEc/van/wpaper";
		} else {
			dir = "";
		}

		if (dir == "") { 
			std::cout << "Could not find existing rdf directory. Returning empty string." << std::endl; 
		} else {
			std::cout << "Returning rdf dir (ID:" + id + "): " + dir << std::endl;
		}
		return dir;
	}

	std::string get_rdf_path(const std::string& id) 
	{
		std::string dir = rdf::get_rdf_dir(id);
		return dir + "/" + id + ".rdf";
	}

	// Expected search criteria: "Volume:" or "Issue:" or "Pages:"
	void update_rdf_line(const std::string& id, const std::string criteria, const std::string new_num_str)
	{
		std::string rdf_path = rdf::get_rdf_path(id);

		std::ifstream input_file(rdf_path);
		if (!input_file.is_open()) {
			std::cerr << "Error (ID: " + id + "): " + "Unable to open file: " + rdf_path << std::endl;
			return;
		}

		// Open the temporary output file for writing
		std::ofstream temp_file("temp.rdf");
		if (!temp_file.is_open()) {
			std::cerr << "Error: Unable to open temporary file temp.rdf" << std::endl;
			input_file.close();
			return;
		}

		std::string line;
		bool found_line = false;

		// Read each line from the file
		while (std::getline(input_file, line)) {
			// Check if the line begins with the criteria string
			if (line.find(criteria) == 0) {
				found_line = true;
				// Write the new line with the updated volume number
				temp_file << criteria + " " + new_num_str << std::endl;
			}
			else {
				// Write the original line to the temporary file
				temp_file << line << std::endl;
			}
		}

		input_file.close();
		temp_file.close();

		// Remove the original file
		if (found_line) {
			std::remove(rdf_path.c_str());
			// Rename the temporary file to the original file name
			std::rename("temp.rdf", rdf_path.c_str());
		}
		else {
			std::cerr << "Error: Line starting with 'Volume: ' not found in file" << std::endl;
			// Remove the temporary file
			std::remove("temp.rdf");
		}
	}
}