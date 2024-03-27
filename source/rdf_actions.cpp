#include "rdf_actions.h"

namespace rdf
{
	std::string get_acronym(const std::string& id, const char delimiter)
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
		// std::cout << "Looking for publication rdf dir (ID:" + id + "): " + pub << std::endl;

		std::array<std::string, 4> acronyms{ "EBFT08","EB","777wps777","VUECON" };

		if (pub.compare(acronyms[0]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/RePEc/EBF/ebfull";
			// Local testing directory only:
			// dir = "C:/Users/work/Desktop/ebfull";
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
			std::cerr << "Could not find existing rdf directory. Returning empty string." << std::endl; 
		} else {
			//std::cout << "Returning rdf dir (ID:" + id + "): " + dir << std::endl;
		}
		return dir;
	}

	std::string get_rdf_path(const std::string& id) 
	{
		std::string path = rdf::get_rdf_dir(id);
		return path + "/" + id + ".rdf";
	}

	// Read from the current state of an rdf into a temp file while updating lines containing the criteria
	void write_to_temp(
		const std::string& rdf_path, 
		const std::string& id, 
		const std::string criteria, 
		const std::string new_str)
	{
		// Open rdf file for read and write access
		std::ifstream read_rdf_file(rdf_path);
		if (!read_rdf_file.is_open()) {
			std::cerr << "Error (ID: " + id + "): " + "Unable to open file for read: " + rdf_path << std::endl;
			return;
		}

		// Create and open a temporary file for write access
		std::ofstream write_temp_file("temp.rdf");
		if (!write_temp_file.is_open()) {
			std::cerr << "Error: Unable to open temporary file temp.rdf for write" << std::endl;
			read_rdf_file.close();
			return;
		}

		std::string line;
		bool found_line = false;

		// Read each line from the file
		while (std::getline(read_rdf_file, line)) {
			// Check if the line begins with the criteria string
			if (line.find(criteria) == 0) {
				found_line = true;
				// Write the new line with the new string
				write_temp_file << criteria + " " + new_str << std::endl;
			}
			else {
				// Write the original line to the temporary file
				write_temp_file << line << std::endl;
			}
		}

		read_rdf_file.close();
		write_temp_file.close();

		if (!(found_line)) {
			std::cerr << "Error: Line starting with '" + criteria + "' not found in file" << std::endl;
			std::remove("temp.rdf");
			throw;
		}
	}

	// Read from a temp file and overwrite the current state of an rdf file
	void read_from_temp(const std::string& rdf_path, const std::string& id)
	{
		// Open rdf file for write access
		std::ofstream write_rdf_file(rdf_path);
		if (!write_rdf_file.is_open()) {
			std::cerr << "Error (ID: " + id + "): " + "Unable to open file for write: " + rdf_path << std::endl;
			return;
		}

		// Open the temporary input file for reading
		std::ifstream read_temp_file("temp.rdf");
		if (!read_temp_file.is_open()) {
			std::cerr << "Error: Unable to open temporary file temp.rdf for read" << std::endl;
			write_rdf_file.close();
			return;
		}

		std::string line;
		// Rewrite the original file to maintain permissions
		while (std::getline(read_temp_file, line)) {
			write_rdf_file << line << std::endl;
		}

		// Close open files
		write_rdf_file.close();
		read_temp_file.close();
		std::remove("temp.rdf");
	}

	// Expected search criteria: "Volume:" or "Issue:" or "Pages:"
	void update_rdf_line(const std::string& id, const std::string criteria, const std::string new_str)
	{
		std::string rdf_path = rdf::get_rdf_path(id);
		try {
			write_to_temp(rdf_path, id, criteria, new_str);
			read_from_temp(rdf_path, id);
		} catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
	}
}