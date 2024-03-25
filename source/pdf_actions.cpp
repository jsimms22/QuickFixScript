#include "pdf_actions.h"
#include "rdf_actions.h"

namespace pdf
{
	std::string get_dir(const std::string& id)
	{
		std::string dir;

		char delimiter = '-';
		std::string pub = rdf::get_acronym(id, delimiter);
		std::cout << "Looking for title page html dir (ID:" + id + "): " + pub << std::endl;

		std::array<std::string, 4> acronyms{ "EBFT08","EB","777wps777","VUECON" };

		if (pub.compare(acronyms[0]) == 0) {
			// dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/EB";
			// Local testing directory only:
			dir = "C:/Users/work/Desktop/EBFT08";
		} else if (pub.compare(acronyms[1]) == 0) {
			// dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/EBFT08";
			// Local testing directory only:
			dir = "C:/Users/work/Desktop/EBFT08";
		} else if (pub.compare(acronyms[2]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/777wps777";
		} else if (pub.compare(acronyms[3]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/VUECON";
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

	std::string get_path(const std::string& id, const pdf::FileType file_type)
	{
		std::string path = pdf::get_dir(id);
		if (file_type == pdf::FileType::PDF) {
			return path + "/" + id + "Pub" + ".pdf";
		}
		else if (file_type == pdf::FileType::HTML) {
			return path + "/" + id + "Pub" + ".html";
		}
		else {
			return "";
		}
	}

	std::string update_title(const std::string& html_content, int new_vol, int new_iss)
	{
		// Define regular expressions for finding volume and issue numbers
		std::regex volume_regex(R"(Volume\s+\d+)");
		std::regex issue_regex(R"(Issue\s+\d+)");

		// Create replacements for volume and issue numbers
		std::string new_title_vol = "Volume " + std::to_string(new_vol);
		std::string new_title_iss = "Issue " + std::to_string(new_iss);

		// Perform replacements
		std::string updated_content = std::regex_replace(html_content, volume_regex, new_title_vol);
		updated_content = std::regex_replace(updated_content, issue_regex, new_title_iss);

		return updated_content;
	}

	std::string update_citation(const std::string& html_content, int new_vol, int new_iss)
	{
		// Define regular expressions for finding volume and issue numbers
		std::regex volume_regex(R"(Vol.\s+\d+)");
		std::regex issue_regex(R"(No.\s+\d+)");
		//std::regex page_regex_1(R"(p.)")

		// Create replacements for volume and issue numbers
		std::string new_cit_vol = "Vol. " + std::to_string(new_vol);
		std::string new_cit_iss = "No. " + std::to_string(new_iss);

		// Perform replacements
		std::string updated_content = std::regex_replace(html_content, volume_regex, new_cit_vol);
		updated_content = std::regex_replace(updated_content, issue_regex, new_cit_iss);

		return updated_content;
	}

	// Updates the stand-alone title page in the html format
	// This does NOT update the publication paper itself
	void update_html(const std::string& id, int new_vol, int new_iss)
	{
		std::string html_path = pdf::get_path(id, pdf::FileType::HTML);

		// Read HTML content from file
		std::ifstream html_file(html_path);
		if (!html_file.is_open()) {
			std::cerr << "Error (ID: " + id + "): " + "Unable to open file: " + html_path << std::endl;
			return;
		}

		std::string html_content((std::istreambuf_iterator<char>(html_file)), std::istreambuf_iterator<char>());
		html_file.close();

		std::string updated_html = pdf::update_title(html_content, new_vol, new_iss);
		updated_html = pdf::update_citation(updated_html, new_vol, new_iss);

		// Write updated HTML content to a temporary file
		std::ofstream temp_file("temp.html");
		if (!temp_file.is_open()) {
			std::cerr << "Error (ID: " + id + "): " + "Unable to open file: " + html_path << std::endl;
			return;
		}

		temp_file << updated_html;
		temp_file.close();

		std::remove(html_path.c_str());
		// Rename the temporary file to the original file name
		std::rename("temp.html", html_path.c_str());
	}

	// Updates the stand-alone title page in the pdf format by converting the updated html title
	// This does NOT update the publication paper itself
	void update_pdf(const std::string& id)
	{
		std::string html_path = pdf::get_path(id, pdf::FileType::HTML);
		std::string pdf_path = pdf::get_path(id, pdf::FileType::PDF);

		std::remove(pdf_path.c_str());

		std::string html_pdf_converter = "c:/inetpub/vhosts/accessecon.com/httpdocs/wkhtmltopdf/bin/wkhtmltopdf.exe";
		std::string command = html_pdf_converter;
		command += html_path + " " + pdf_path;

		int result = system(command.c_str());
		if (result == 0) {
			std::cout << "HTML file converted to PDF successfully." << std::endl;
		}
		else {
			std::cerr << "Error (ID: " + id + "): " + "Failed to convert HTML file to PDF." << std::endl;
			return;
		}
	}


}