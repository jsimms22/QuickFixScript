#include "file_actions.h"

namespace file
{
    // Verifies the directory exists
    bool directory_exists(const std::string& dir) { return fs::is_directory(dir); }

    // Verifiesd the filetype is .pdf
    bool is_pdf(const std::string& filename) {
        // Convert the filename to lowercase for case-insensitive comparison
        std::string lowercaseFilename = filename;
        std::transform(lowercaseFilename.begin(), lowercaseFilename.end(), lowercaseFilename.begin(), ::tolower);

        // Check if the filename ends with ".pdf"
        return lowercaseFilename.size() >= 4 &&
            lowercaseFilename.substr(lowercaseFilename.size() - 4) == ".pdf";
    }

    // Fills a vector with filenames entries for every .pdf in a directory
    void build_file_vec(const std::string& dir, std::vector<fs::directory_entry>& file_vec)
    {
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry) && is_pdf(entry.path().filename().string())) {
                file_vec.push_back(entry);
            }
        }
    }

    // Uses regex to determine proper numerical order and avoids cases where -P9 is ordered after -P10, etc
    bool compare_filenames(const std::filesystem::directory_entry& entry1, const std::filesystem::directory_entry& entry2) 
    {
        // Extract filenames
        std::string filename1 = entry1.path().filename().string();
        std::string filename2 = entry2.path().filename().string();

        // Define regex pattern to match the numeric part after "P"
        std::regex pattern(R"((\d+)(?:\.pdf)$)");

        // Extract numeric parts from filenames
        std::smatch match1, match2;
        std::regex_search(filename1, match1, pattern);
        std::regex_search(filename2, match2, pattern);

        // Convert matched strings to integers
        int number1 = std::stoi(match1[1]);
        int number2 = std::stoi(match2[1]);

        // Compare numbers
        return number1 < number2;
    }

    // Deterministically orders the files in vector by their paper number
    void sort_files(std::vector<std::filesystem::directory_entry>& entries) 
    {
        std::sort(entries.begin(), entries.end(), compareFilenames);
    }

    // Renames the entry's filename in filesystem with the a new year, volume, and issue number
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

    // Renames the entry's filename in filesystem with the a new year, volume, issue, and paper number
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

    // Renames a temp string for updating fields in a database since the entry class is non-mutable
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

    // Renames a temp string for updating fields in a database since the entry class is non-mutable, overloads to update paper number
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