#include "pdf_actions.h"
#include "rdf_actions.h"

namespace pdf
{
	std::string get_dir(const std::string& id)
	{
		std::string dir;

		char delimiter = '-';
		std::string pub = rdf::get_acronym(id, delimiter);
		//std::cout << "Looking for title page html dir (ID:" + id + "): " + pub << std::endl;

		std::array<std::string, 4> acronyms{ "EBFT08","EB","777wps777","VUECON" };

		if (pub.compare(acronyms[0]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/EBFT08";
			// Local testing directory only:
			// dir = "C:/Users/work/Desktop/EBFT08";
		} else if (pub.compare(acronyms[1]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/EB";
			// Local testing directory only:
			// dir = "C:/Users/work/Desktop/EB";
		} else if (pub.compare(acronyms[2]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/777wps777";
		} else if (pub.compare(acronyms[3]) == 0) {
			dir = "C:/inetpub/vhosts/accessecon.com/httpdocs/pubs/VUECON";
		} else {
			dir = "";
		}

		if (dir == "") {
			std::cerr << "Could not find existing html directory. Returning empty string." << std::endl;
		} else {
			//std::cout << "Returning html dir (ID:" + id + "): " + dir << std::endl;
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

	std::string update_title(const std::string& html_content, const int new_vol, const int new_iss)
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

	std::string update_citation(const std::string& html_content, const int new_vol, const int new_iss)
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
	void update_html(const std::string& id, const int new_vol, const int new_iss)
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

		// Using a 3rd party open source html->pdf converter called wkhtmltopdf on the cmd line
		std::string html_pdf_converter = "C:/inetpub/vhosts/accessecon.com/httpdocs/wkhtmltopdf/bin/wkhtmltopdf.exe";
		std::string cmd = html_pdf_converter;
		cmd += " " + html_path + " " + pdf_path;

		int result = system(cmd.c_str());
		// For debugging purposes
		// std::cout << cmd << std::endl;
		// int result = 0;
		// system() will return 0 if successful, -1 if failed
		if (result == 0) {
			std::cout << "HTML file converted to PDF successfully." << std::endl;
		} else {
			std::cerr << "Error (ID: " + id + "): " + "Failed to convert HTML file to PDF." << std::endl;
			return;
		}
	}

	std::string get_pub_paper_path(const fs::directory_entry entry, const std::string& id, const std::string filename)
	{
		std::string pdf_path;
		if (fs::exists(entry.path())) {
			pdf_path = entry.path().parent_path().string();
			pdf_path += "/" + filename;
		} else {
			// For debugging:
			// pdf_path = pdf::get_dir(id);
			// pdf_path += "/2024/Volume0/" + filename;
		}
		return pdf_path;
	}

	void remove_title_page(const fs::directory_entry entry, const std::string& id, const std::string filename)
	{
		std::string pub = rdf::get_acronym(id, '-');
		std::string base_path = pdf::get_dir(id);
		base_path += "/GeneralPDF" + pub;

		// Mimicking the naming conventions of the paperGenerator.php script used by the 
		// website: pdf_out <-> pdfOut) and temp_pdf_in <-> tempPdfIn
		std::string pdf_out = pdf::get_pub_paper_path(entry, id, filename);

		bool copied = false;
		if (fs::is_regular_file(pdf_out.c_str())) {
			std::string temp_pdf_in = base_path + "/" + id + "finalPaper_ScriptFix.pdf";

			// Creating a copy of the published paper
			if (fs::exists(temp_pdf_in)) {
				copied = fs::copy_file(pdf_out, temp_pdf_in, fs::copy_options::overwrite_existing);
			} else {
				copied = fs::copy_file(pdf_out, temp_pdf_in, fs::copy_options::none);
			}

			if (!copied) {
				std::cerr << "Error (ID:" + id + "): Did not create copy of " << pdf_out << std::endl;
				if (fs::exists(temp_pdf_in)) { fs::remove(temp_pdf_in); }
				else { return; }
			} else {
				// Run the ghostscript exe that is already used by the server
				std::string ghost_script_bin = "C:/inetpub/vhosts/accessecon.com/httpdocs/ghostscript/bin/gswin32c.exe";
				std::string cmd_options = "-dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -dFirstPage=2 -sOutputFile=";
				std::string cmd = ghost_script_bin + " " + cmd_options + pdf_out + " " + temp_pdf_in;

				int result = std::system(cmd.c_str());
				// For debugging:
				// std::cout << cmd << std::endl;
				// int result = 0;
				// system() will return 0 if successful, -1 if failed
				if (result == 0) {
					std::cout << "Old title page successfully removed from the published PDF." << std::endl;
				} else {
					std::cerr << "Error (ID: " + id + "): " + "Failed to remove old title page." << std::endl;
					return;
				}
			}
		} else {
			std::cerr << "Unexpected filetype, returning without removing original title page." << std::endl;
			return;
		}
	}

	void update_title_page(const fs::directory_entry entry, const std::string& id, const std::string filename)
	{
		std::string pub = rdf::get_acronym(id, '-');
		std::string base_path = pdf::get_dir(id);
		// Mimicking the naming conventions of the paperGenerator.php script used by the 
		// website: pdf_out <-> pdfOut) and temp_pdf_in <-> tempPdfIn
		std::string temp_pdf_in =base_path + "/GeneralPDF" + pub;
		temp_pdf_in += "/" + id + "_finalPaper_ScriptFix2.pdf";
		std::string title_page_pdf = base_path + "/" + id + "Pub.pdf";
		std::string pdf_out = pdf::get_pub_paper_path(entry, id, filename);

		bool copied = false;
		if (fs::is_regular_file(pdf_out.c_str())) {
			// Creating a copy of the published paper
			if (fs::exists(temp_pdf_in)) {
				copied = fs::copy_file(pdf_out, temp_pdf_in, fs::copy_options::overwrite_existing);
			} else {
				copied = fs::copy_file(pdf_out, temp_pdf_in, fs::copy_options::none);
			}

			if (!copied) {
				std::cerr << "Error (ID:" + id + "): Did not create copy of " << pdf_out << std::endl;
				if (fs::exists(temp_pdf_in)) { fs::remove(temp_pdf_in); }
				else { return; }
			}
			else {
				// Run the ghostscript exe that is already used by the server
				std::string ghost_script_bin = "C:/inetpub/vhosts/accessecon.com/httpdocs/ghostscript/bin/gswin32c.exe";
				std::string cmd_options = "-dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -dPDFSETTINGS=/prepress -sOutputFile=";
				std::string cmd = ghost_script_bin + " " + cmd_options + pdf_out + " " + title_page_pdf + " " + temp_pdf_in;

				int result = std::system(cmd.c_str());
				// For debugging:
				// std::cout << cmd << std::endl;
				// int result = 0;
				// system() will return 0 if successful, -1 if failed
				if (result == 0) {
					std::cout << "New title page successfully added to the published PDF." << std::endl;
				}
				else {
					std::cerr << "Error (ID: " + id + "): " + "Failed to remove old title page." << std::endl;
					return;
				}
			}
		} else {
			std::cerr << "Unexpected filetype, returning without adding new title page." << std::endl;
			return;
		}
	}
}