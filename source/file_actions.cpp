#include "file_actions.h"

namespace file
{
    bool directory_exists(const std::string& dir) { return fs::is_directory(dir); }

    bool is_pdf(const std::string& filename) {
        // Convert the filename to lowercase for case-insensitive comparison
        std::string lowercaseFilename = filename;
        std::transform(lowercaseFilename.begin(), lowercaseFilename.end(), lowercaseFilename.begin(), ::tolower);

        // Check if the filename ends with ".pdf"
        return lowercaseFilename.size() >= 4 &&
            lowercaseFilename.substr(lowercaseFilename.size() - 4) == ".pdf";
    }

    void build_file_array(const std::string& dir, std::vector<fs::directory_entry>& file_vec)
    {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry) && is_pdf(entry.path().filename().string())) {
                file_vec.push_back(entry);
            }
        }
    }

    void rename_file(const fs::directory_entry& entry, const int new_vol, const int new_iss)
    {
        assert(new_vol >= 0);
        assert(new_iss >= 0);

        std::regex pattern1("-V(\\d+)");
        std::regex pattern2("-I(\\d+)");
        std::regex pattern3("-(\\d+)-");

        std::string year = std::to_string(new_vol + 2000 - 20);

        if (fs::is_regular_file(entry)) {
            std::string oldFileName = entry.path().filename().string();
            std::cout << "Changing filename locally: " + oldFileName + " -> ";
            std::smatch match;

            if (std::regex_search(oldFileName, match, pattern1) && 
                std::regex_search(oldFileName, match, pattern2) &&
                std::regex_search(oldFileName, match, pattern3)) {
                // Construct the new file name
                std::string newFileName = std::regex_replace(oldFileName, pattern1, "-V" + std::to_string(new_vol));
                newFileName = std::regex_replace(newFileName, pattern2, "-I" + std::to_string(new_iss));
                newFileName = std::regex_replace(newFileName, pattern3, "-" + year.substr(2,2) + "-");

                // Rename the file
                fs::rename(entry.path(), entry.path().parent_path() / newFileName);

                std::cout << newFileName << std::endl;
            }
        } else {
            std::cerr << "Unexpected filetype, returning without updating filename." << std::endl;
            return;
        }
    }

    void rename_file(const fs::directory_entry& entry, const int new_vol, const int new_iss, const int new_paper_num)
    {
        assert(new_vol >= 0);
        assert(new_iss >= 0);
        assert(new_paper_num >= 0);

        std::regex pattern1("-V(\\d+)");
        std::regex pattern2("-I(\\d+)");
        std::regex pattern3("-P(\\d+)");
        std::regex pattern4("-(\\d+)-");

        std::string year = std::to_string(new_vol + 2000 - 20);

        if (fs::is_regular_file(entry)) {
            std::string oldFileName = entry.path().filename().string();
            std::cout << "Changing filename locally: " + oldFileName + " -> ";
            std::smatch match;

            if (std::regex_search(oldFileName, match, pattern1) && 
                std::regex_search(oldFileName, match, pattern2) &&
                std::regex_search(oldFileName, match, pattern3) &&
                std::regex_search(oldFileName, match, pattern4)) {
                // Construct the new file name
                std::string newFileName = std::regex_replace(oldFileName, pattern1, "-V" + std::to_string(new_vol));
                newFileName = std::regex_replace(newFileName, pattern2, "-I" + std::to_string(new_iss));
                newFileName = std::regex_replace(newFileName, pattern3, "-P" + std::to_string(new_paper_num));
                newFileName = std::regex_replace(newFileName, pattern4, "-" + year.substr(2, 2) + "-");

                // Rename the file
                fs::rename(entry.path(), entry.path().parent_path() / newFileName);

                std::cout << newFileName << std::endl;
            }
        }
        else {
            std::cerr << "Unexpected filetype, returning without updating filename." << std::endl;
            return;
        }
    }

    void rename_temp_filename(std::string& temp_filename, const int new_vol, const int new_iss)
    {
        assert(new_vol >= 0);
        assert(new_iss >= 0);

        std::regex pattern1("-V(\\d+)");
        std::regex pattern2("-I(\\d+)");
        std::regex pattern3("-(\\d+)-");

        std::string year = std::to_string(new_vol + 2000 - 20);

        std::string oldFileName = temp_filename;
        std::smatch match;

        if (std::regex_search(temp_filename, match, pattern1) && 
            std::regex_search(temp_filename, match, pattern2) && 
            std::regex_search(temp_filename, match, pattern3)) {
            // Construct the new file name
            temp_filename = std::regex_replace(oldFileName, pattern1, "-V" + std::to_string(new_vol));
            temp_filename = std::regex_replace(temp_filename, pattern2, "-I" + std::to_string(new_iss));
            temp_filename = std::regex_replace(temp_filename, pattern3, "-" + year.substr(2, 2) + "-");
        }
    }

    void rename_temp_filename(std::string& temp_filename, const int new_vol, const int new_iss, const int new_paper_num)
    {
        assert(new_vol >= 0);
        assert(new_iss >= 0);
        assert(new_paper_num >= 0);

        std::regex pattern1("-V(\\d+)");
        std::regex pattern2("-I(\\d+)");
        std::regex pattern3("-P(\\d+)");
        std::regex pattern4("-(\\d+)-");

        std::string year = std::to_string(new_vol + 2000 - 20);

        std::string old_filename = temp_filename;
        std::smatch match;

        if (std::regex_search(temp_filename, match, pattern1) && 
            std::regex_search(temp_filename, match, pattern2) &&
            std::regex_search(temp_filename, match, pattern3) &&
            std::regex_search(temp_filename, match, pattern4)) {
            // Construct the new file name
            temp_filename = std::regex_replace(old_filename, pattern1, "-V" + std::to_string(new_vol));
            temp_filename = std::regex_replace(temp_filename, pattern2, "-I" + std::to_string(new_iss));
            temp_filename = std::regex_replace(temp_filename, pattern3, "-P" + std::to_string(new_paper_num));
            temp_filename = std::regex_replace(temp_filename, pattern4, "-" + year.substr(2, 2) + "-");
        }
    }
}