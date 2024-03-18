#include "sql_agent.h"
#include "file_renamer.h"
#include "sql_actions.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) 
{
    /* Testing and capturing .exe inputs */
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <new_volume_number> <new_issue_number> <db_schema> <root> <password>" << std::endl;
        return 1;
    }
    std::string directoryPath = argv[1];
    // Check if the directory exists
    if (!file::directory_exists(directoryPath)) {
        std::cerr << "Error: Directory does not exist." << std::endl;
        return 1;
    }
    // Check if the new volume number makes sense
    int newVolumeNum = std::stoi(argv[2]);
    if (newVolumeNum < 20 || newVolumeNum >= 100) {
        std::cerr << "Error: invalid new volume number. Must be in the range of [20,100)." << std::endl;
        return 1;
    }
    // Check if the new issue number makes sense
    int newIssueNum = std::stoi(argv[3]);
    if (newIssueNum < 0 || newIssueNum >= 5) {
       std::cerr << "Error: invalid new issue number. Must be in the range of [0,5)." << std::endl;
       return 1;
    }
    // Convert integers to strings for updating the database for an entry
    std::string year_str = std::to_string((newVolumeNum - 20) + 2000);
    std::string vol_str = std::to_string(newVolumeNum);
    std::string iss_str = std::to_string(newIssueNum);

    std::string schema = argv[4];
    std::string username = argv[5];
    std::string password = argv[6];

    /* Build MySQL Interface and try to connect to the DB Server */
    sql_agent::MySQL_Interface mysql_db;
    mysql_db.set_driver();
    mysql_db.set_server(sql_agent::Protocol::TCP, "127.0.0.1", "3306");
    mysql_db.set_user(username, password);
    mysql_db.set_schema(schema);
    try {
        mysql_db.set_connection();
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    /* Build array of all .pdf files in array, and verify they match the expected naming convention */
    std::vector<fs::directory_entry> file_vec;
    file::build_file_array(directoryPath, file_vec);

    /* Use their path to retrieve each file's ID from the database, then execute updates */
    sql::Statement* query = mysql_db.get_connection()->createStatement();
    sql::ResultSet* result = nullptr;
    
    /* Loop operations
    * 1) Verify the file entry is acceptable to use
    * 2) Reset loop variables
    * 3) Retrieve the ID of the file from the database, if we find an ID do the following:
    *   3.1) Execute query to UPDATE Volume_Number for the given ID
    *   3.2) Execute query to UPDATE NumIssue for the given ID
    *   3.3) Execute query to UPDATE citationString for the given ID
    *   3.4) Execute query to UPDATE Published_PDF_File for the given ID
    *       3.4.1) If we updated the published pdf filepath then set pub_path_updated = true;
    * 4) If pub_path_updated = true then,
    *   4.1) Update the filename in file explorer to match the new Published_PDF_File field in the database
    */
    for (const auto& entry : file_vec) {
        if (!fs::is_regular_file(entry) && !file::is_pdf(entry.path().filename().string())) continue;

        std::cout << "\nWorking on: " << entry.path().filename().string() << std::endl;
        std::string temp_filename = entry.path().filename().string();

        // Initialize and reset result_id
        std::string result_id = "";

        // Keeps us from updating the local filename if the published pdf path is not updated in the mysql DB
        bool pub_path_updated = false;

        /* SQL DATABASE UPDATING */
        try { 
            // Execute a SELECT query to retrieve ID field for the entry
            result_id = sql_agent::retrieve_field(query, result, temp_filename, "ID");
            std::cout << "Retrieved ID: " + result_id << std::endl;

            if (result_id != "") {
                std::cout << "Starting SQL Database Updates for ID: " + result_id << std::endl;

                // Constructing new volume string
                std::string new_vol_str = year_str + vol_str + "000" + iss_str;
                std::cout << "New Volume Number (ID: " + result_id + "): " + new_vol_str << std::endl;
                // Execute query to UPDATE Volume_Number for the given ID
                sql_agent::update_field_by_ID(query, result_id, "Volume_Number", new_vol_str);

                // Execute query to UPDATE NumIssue for the given ID
                std::cout << "New Issue Number (ID: " + result_id + "): " + iss_str << std::endl;
                sql_agent::update_field_by_ID(query, result_id, "NumIssue", iss_str);

                // Constructing new citiation string
                std::string new_citationString = year_str + ", Volume " + vol_str + ", Issue " + iss_str;
                std::string last_page = sql_agent::retrieve_field(query, result, temp_filename, "TotalNumpages");
                std::string page_count = sql_agent::retrieve_field(query, result, temp_filename, "NumberOfPages");
                if (last_page != "" && page_count != "") {
                    int first_page_num = std::stoi(last_page) - std::stoi(page_count) + 1;
                    new_citationString += ", pages " + std::to_string(first_page_num) + " - " + last_page;
                }
                // Execute query to UPDATE citationString for the given ID
                std::cout << "New Citation String (ID: " + result_id + "): " + new_citationString << std::endl;
                sql_agent::update_field_by_ID(query, result_id, "citationString", new_citationString);

                // Updating the temp copy of the filename to update the DB
                file::rename_temp_filename(temp_filename, newVolumeNum, newIssueNum);
                std::string new_Published_PDF_File = "/Pubs/EBFT08/" + year_str + "/Volume" + vol_str + "/" + temp_filename;
                std::cout << "New Published PDF Filepath (ID: " + result_id + "): " + new_Published_PDF_File << std::endl;
                // Execute query to UPDATE Published_PDF_File for the given ID
                sql_agent::update_field_by_ID(query, result_id, "Published_PDF_File", new_Published_PDF_File);
                pub_path_updated = true;

                std::cout << "Successfully Updated SQL Database for ID: " << result_id << std::endl;
            } else {
                std::cerr << "Retrieved empty ID string, moving to next file." << std::endl;
            }
        } catch (sql::SQLException& e) {
            std::cerr << "Query error: " << e.what() << std::endl;
            std::cerr << "Failed to update all SQL fields for ID: " + result_id << std::endl;
            continue;
        }

        /* UPDATING FILE IN THE FILE EXPLORER */
        try {
            if (pub_path_updated) {
                file::rename_file(entry, newVolumeNum, newIssueNum);
            } else {
                std::cout << "Skipped updating the filename locally." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cerr << "Failed to update filename: " + entry.path().string();
            continue;
        }
    }

    delete query;
    delete result;

    return 0;
}

// Notes
/*
// for back dating the publication date
-[If Needed] Publish Date
// possibly required, too early to tell
- [Maybe] Current_Status
- [Maybe] Content_ID
*/