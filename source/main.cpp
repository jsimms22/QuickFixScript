#include "sql_agent.h"
#include "file_actions.h"
#include "sql_actions.h"
#include "rdf_actions.h"
#include "pdf_actions.h"
#include <chrono>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) 
{
    /* Testing and capturing .exe inputs */
    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " <directory_path> <new_volume_number> <new_issue_number> <last_article_number> <db_schema_name> <username> <password>" << std::endl;
        return 1;
    }
    std::string directoryPath = argv[1];
    // Check if the directory exists
    if (!file::directory_exists(directoryPath)) {
        std::cerr << "Error: Directory does not exist." << std::endl;
        return 1;
    }
    int newVolumeNum = std::stoi(argv[2]);
    // Check if the new volume number makes sense
    if (newVolumeNum < 20 || newVolumeNum >= 100) {
        std::cerr << "Error: invalid new volume number. Must be in the range of [20,99]." << std::endl;
        return 1;
    }
    int newIssueNum = std::stoi(argv[3]);
    // Check if the new issue number makes sense
    if (newIssueNum < 0 || newIssueNum >= 10) {
       std::cerr << "Error: invalid new issue number. Must be in the range of [0,9]." << std::endl;
       return 1;
    }
    // This should be initialized to the last paper's sequence number published in a volume
    // For example: EB-V44-I1-P26, newPaperNum should be set to 26
    int newPaperNum = std::stoi(argv[4]);
    if (newPaperNum < 0) {
        std::cerr << "Error: invalid last article number. Should be equivalent to the sequence number for the last paper published to the desired volume." << std::endl;
        std::cerr << "For example: if the last published article in volume 44 has the filename 'V44-I1-P26', ";
        std::cerr << "then last article number should be set to 26." << std::endl;
        std::cerr << "If this is a new volume with no prior issues then set last article number to 0." << std::endl;
        return 1;
    }
    std::string schema = argv[5];
    std::string username = argv[6];
    std::string password = argv[7];

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

    // Convert integers to strings for updating the database for an entry
    std::string year_str = std::to_string((newVolumeNum - 20) + 2000);
    std::string vol_str = std::to_string(newVolumeNum);
    std::string iss_str = std::to_string(newIssueNum);

    bool prev_published_paper = false;
    // Check assumed last published paper and initial newPaperNum passes basic sanity checks
    std::string l_paper_pub; 
    std::string l_paper_dir; 
    std::string l_paper_pdf_path; 
    std::string l_pub_id;
    if (newPaperNum != 0) {
        l_paper_pub = rdf::get_acronym(file_vec[0].path().filename().string(), '-');
        l_paper_dir = "/Pubs/" + l_paper_pub + "/" + year_str + "/Volume" + vol_str;
        try {
            l_paper_pdf_path = sql_agent::retrieve_field(query, result, std::to_string(newPaperNum), l_paper_dir, "Published_PDF_File");
            l_pub_id = sql_agent::retrieve_field(query, result, l_paper_pdf_path, "ID");
        } catch (const sql::SQLException& e) {
            std::cerr << "Query error: " << e.what() << std::endl;
            std::cerr << "Could not find an ID for that last paper published in Volume " + vol_str << std::endl;
            return 1;
        }
        std::cout << "Deduced last published paper in " + year_str + ", volume " + vol_str + " is: " + l_paper_pdf_path << std::endl;

        std::string l_pub_dir = pdf::get_dir(l_pub_id);
        std::string l_pub_full_path = l_pub_dir + l_paper_pdf_path;
        if (fs::exists(l_pub_full_path)) {
            std::cout << "Found: " + l_pub_full_path << std::endl;
        }
        else {
            std::cerr << "Error: Expected file at location " + l_pub_full_path + " to exist"; 
            std::cout << " and contain the last published paper in the targeted Volume " + vol_str << std::endl;
            // Remove for debugging:
            //return 1;
        }
        prev_published_paper = true;
    } else {
        std::cout << "Continuing with script and assuming no previously published papers exist in the targeted volume." << std::endl;
    }
    
    /* LOOP OPERATION OVERVIEW
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
    * 5) If local_path_update = true then,
    *   5.1) Update the associated rdf
    * 6) If local_path_updated = true && rdf_updated = true
    *   6.1) Update the title page for published paper entry
    */
    if (prev_published_paper) {
        std::string last_paper_id = l_pub_id;
        std::string last_pub_page = sql_agent::retrieve_field(query, result, l_paper_pdf_path, "TotalNumpages");
        newPaperNum += 1;
        std::string paper_num_str = std::to_string(newPaperNum);

        for (const auto& entry : file_vec) {
            if (!fs::is_regular_file(entry) && !file::is_pdf(entry.path().filename().string())) continue;

            std::cout << "\nWorking on: " << entry.path().filename().string() << std::endl;
            std::string temp_filename = entry.path().filename().string();

            // Initialize and reset result_id
            std::string result_id = "";

            // Keeps us from updating the local filename if the published pdf path is not updated in the mysql DB
            bool db_path_updated = false;
            bool local_path_updated = false;
            bool rdf_updated = false;

            // Reset page range tracker array
            std::array<std::string, 2> page_range{ "","" };
            // Reset extracted pub acronym
            std::string pub = "";

            /* UPDATING SQL DATABASE FOR PUBLISHED PAPER */
            try {
                // Execute a SELECT query to retrieve ID field for the entry
                result_id = sql_agent::retrieve_field(query, result, temp_filename, "ID");
                std::cout << "Retrieved ID: " + result_id << std::endl;

                if (result_id != "") {
                    std::cout << "Starting SQL Database Updates for ID: " + result_id << std::endl;
                    pub = rdf::get_acronym(result_id, '-');

                    // Constructing new volume string
                    std::string new_vol_str = year_str + vol_str + "000" + iss_str;
                    std::cout << "New Volume Number (ID: " + result_id + "): " + new_vol_str << std::endl;
                    // Execute query to UPDATE Volume_Number for the given ID
                    sql_agent::update_field_by_ID(query, result_id, "Volume_Number", new_vol_str);

                    // Execute query to UPDATE NumIssue for the given ID
                    std::cout << "New Issue Number (ID: " + result_id + "): " + iss_str << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "NumIssue", iss_str);

                    // Execute query to UPDATE TotalPaper for the given ID
                    std::cout << "New Paper Number (ID: " + result_id + "): " + paper_num_str << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "TotalPaper", paper_num_str);

                    // Constructing new citiation string
                    std::string new_citationString = year_str + ", Volume " + vol_str + ", Issue " + iss_str;
                    std::string page_count = sql_agent::retrieve_field(query, result, temp_filename, "NumberOfPages");
                    page_range[1] = std::to_string(std::stoi(last_pub_page) + std::stoi(page_count));
                    if (page_range[1] != "" && page_count != "") {
                        int first_page_num = std::stoi(page_range[1]) - std::stoi(page_count) + 1;
                        page_range[0] = std::to_string(first_page_num);
                        // Execute query to UPDATE TotalNumpages for the given ID
                        sql_agent::update_field_by_ID(query, result_id, "TotalNumpages", page_range[1]);
                        if ((std::stoi(page_range[1]) - std::stoi(page_range[0])) == 0) {
                            new_citationString += ", pages " + page_range[1];
                        } else {
                            new_citationString += ", pages " + page_range[0] + " - " + page_range[1];
                        }
                    }
                    // Execute query to UPDATE citationString for the given ID
                    std::cout << "New Citation String (ID: " + result_id + "): " + new_citationString << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "citationString", new_citationString);

                    // Updating the temp copy of the filename to update the DB
                    file::rename_temp_filename(temp_filename, newVolumeNum, newIssueNum, newPaperNum);
                    std::string new_Published_PDF_File = "/Pubs/" + pub + "/" + year_str + "/Volume" + vol_str + "/" + temp_filename;
                    std::cout << "New Published PDF Filepath (ID: " + result_id + "): " + new_Published_PDF_File << std::endl;
                    // Execute query to UPDATE Published_PDF_File for the given ID
                    sql_agent::update_field_by_ID(query, result_id, "Published_PDF_File", new_Published_PDF_File);

                    // Build calendar stamp for new publish date 
                    std::string new_Publish_Date = year_str + "-12-31";
                    // Get current time for the timestamp and convert to std::string format
                    std::time_t current_time = std::time(nullptr);
                    char buffer[20];
                    std::strftime(buffer, sizeof(buffer), "%T", std::localtime(&current_time));
                    std::string time_str = std::string(buffer);
                    new_Publish_Date += " " + time_str;
                    // Execute query to UPDATE Publish_Date for the given ID
                    std::cout << "New Published Date (ID: " + result_id + "): " + new_Publish_Date << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "Publish_Date", new_Publish_Date);

                    db_path_updated = true;
                    std::cout << "Successfully Updated SQL Database for ID: " << result_id << std::endl;
                } else {
                    std::cerr << "Retrieved empty ID string, moving to next file." << std::endl;
                }
            } catch (const sql::SQLException& e) {
                std::cerr << "Query error: " << e.what() << std::endl;
                std::cerr << "Failed to update all SQL fields for ID: " + result_id << std::endl;
                continue;
            }

            /* UPDATING FILENAME FOR PUBLISHED PAPER */
            try {
                if (db_path_updated) {
                    // Update filename
                    file::rename_file(entry, newVolumeNum, newIssueNum, newPaperNum);
                    local_path_updated = true;
                } else {
                    std::cout << "Skipped updating the filename locally." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update filename: " + entry.path().string() << std::endl;
                continue;
            }

            /* UPDATING RDF CONTENTS FOR PUBLISHED PAPER */
            try {
                if (local_path_updated) {
                    // Updates RDF, uses the ID to find associated rdf 
                    // then finds line containing the given criteria with the given string
                    std::string new_url = "http://";
                    new_url += "www.accessecon.com/Pubs/";
                    new_url += pub + "/" + year_str + "/Volume" + vol_str + "/" + temp_filename;
                    std::string new_creation_date = year_str + "-12-31";

                    // Update rdf for each line that contains the following fields
                    rdf::update_rdf_line(result_id, "Creation-Date:", new_creation_date);
                    rdf::update_rdf_line(result_id, "File-URL:", new_url);
                    if ((std::stoi(page_range[1]) - std::stoi(page_range[0])) == 0) {
                        rdf::update_rdf_line(result_id, "Pages:", page_range[1]);
                    } else {
                        rdf::update_rdf_line(result_id, "Pages:", page_range[0] + " - " + page_range[1]);
                    }
                    rdf::update_rdf_line(result_id, "Year:", year_str);
                    rdf::update_rdf_line(result_id, "Volume:", vol_str);
                    rdf::update_rdf_line(result_id, "Issue:", iss_str);

                    rdf_updated = true;
                } else {
                    std::cout << "Skipped updating the rdf contents." << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update all rdf contents for ID: " + result_id << std::endl;
                continue;
            }

            /* UPDATING HTML AND PDF TITLE PAGES FOR PUBLISHED PAPER */
            try {
                if (local_path_updated && rdf_updated) {
                    // Updates the stand-alone html title page (if it exists)
                    pdf::update_html(result_id, newVolumeNum, newIssueNum, page_range);
                    // Overwrites existing stand-alone pdf title page with updated html version
                    pdf::update_pdf(result_id);
                    // Removes the current title page from a published paper
                    pdf::remove_title_page(entry, result_id, temp_filename);
                    // Cats the title created during update_pdf() with the original published paper (minus old title) 
                    pdf::update_title_page(entry, result_id, temp_filename);
                } else {
                    std::cout << "Skipped updating with new title page." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update title page for ID: " + result_id << std::endl;
                continue;
            }

            // Setting current iterated entry to be last published entry
            if (db_path_updated && local_path_updated && rdf_updated) {
                last_paper_id = result_id;
                last_pub_page = sql_agent::retrieve_field(query, result, temp_filename, "TotalNumpages");
            }
        }
    } else {
        for (const auto& entry : file_vec) {
            if (!fs::is_regular_file(entry) && !file::is_pdf(entry.path().filename().string())) continue;

            std::cout << "\nWorking on: " << entry.path().filename().string() << std::endl;
            std::string temp_filename = entry.path().filename().string();

            // Initialize and reset result_id
            std::string result_id = "";
            // Keeps us from updating the local filename if the published pdf path is not updated in the mysql DB
            bool db_path_updated = false;
            bool local_path_updated = false;
            bool rdf_updated = false;
            // Reset page range tracker array
            std::array<std::string, 2> page_range{"",""};
            // Reset extracted pub acronym
            std::string pub = "";

            /* UPDATING SQL DATABASE FOR PUBLISHED PAPER */
            try {
                // Execute a SELECT query to retrieve ID field for the entry
                result_id = sql_agent::retrieve_field(query, result, temp_filename, "ID");
                std::cout << "Retrieved ID: " + result_id << std::endl;

                if (result_id != "") {
                    std::cout << "Starting SQL Database Updates for ID: " + result_id << std::endl;
                    pub = rdf::get_acronym(result_id, '-');

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
                    page_range[1] = sql_agent::retrieve_field(query, result, temp_filename, "TotalNumpages");
                    std::string page_count = sql_agent::retrieve_field(query, result, temp_filename, "NumberOfPages");
                    if (page_range[1] != "" && page_count != "") {
                        int first_page_num = std::stoi(page_range[1]) - std::stoi(page_count) + 1;
                        page_range[0] = std::to_string(first_page_num);
                        if ((std::stoi(page_range[1]) - std::stoi(page_range[0])) == 0) {
                            new_citationString += ", pages " + page_range[1];
                        } else {
                            new_citationString += ", pages " + page_range[0] + " - " + page_range[1];
                        }
                    }
                    // Execute query to UPDATE citationString for the given ID
                    std::cout << "New Citation String (ID: " + result_id + "): " + new_citationString << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "citationString", new_citationString);

                    // Updating the temp copy of the filename to update the DB
                    file::rename_temp_filename(temp_filename, newVolumeNum, newIssueNum);
                    std::string new_Published_PDF_File = "/Pubs/" + pub + "/" + year_str + "/Volume" + vol_str + "/" + temp_filename;
                    std::cout << "New Published PDF Filepath (ID: " + result_id + "): " + new_Published_PDF_File << std::endl;
                    // Execute query to UPDATE Published_PDF_File for the given ID
                    sql_agent::update_field_by_ID(query, result_id, "Published_PDF_File", new_Published_PDF_File);

                    // Build new publish date string
                    std::string new_Publish_Date = year_str + "-12-31";
                    // Get current time for the timestamp and convert to std::string format
                    std::time_t current_time = std::time(nullptr);
                    char buffer[20];
                    std::strftime(buffer, sizeof(buffer), "%T", std::localtime(&current_time));
                    std::string time_str = std::string(buffer);
                    new_Publish_Date += " " + time_str;
                    // Execute query to UPDATE Publish_Date for the given ID
                    std::cout << "New Published Date (ID: " + result_id + "): " + new_Publish_Date << std::endl;
                    sql_agent::update_field_by_ID(query, result_id, "Publish_Date", new_Publish_Date);

                    db_path_updated = true;
                    std::cout << "Successfully Updated SQL Database for ID: " << result_id << std::endl;
                } else {
                    std::cerr << "Retrieved empty ID string, moving to next file." << std::endl;
                }
            } catch (const sql::SQLException& e) {
                std::cerr << "Query error: " << e.what() << std::endl;
                std::cerr << "Failed to update all SQL fields for ID: " + result_id << std::endl;
                continue;
            }

            /* UPDATING FILENAME FOR PUBLISHED PAPER */
            try {
                if (db_path_updated) {
                    // Update filename
                    file::rename_file(entry, newVolumeNum, newIssueNum);
                    local_path_updated = true;
                } else {
                    std::cout << "Skipped updating the filename locally." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update filename: " + entry.path().string() << std::endl;
                continue;
            }

            /* UPDATING RDF CONTENTS FOR PUBLISHED PAPER */
            try {
                if (local_path_updated) {
                    // Updates RDF, uses the ID to find associated rdf 
                    // then finds line containing the given criteria with the given string
                    std::string new_url = "http://";
                    new_url += "www.accessecon.com/Pubs/";
                    new_url += pub + "/" + year_str + "/Volume" + vol_str + "/" + temp_filename;
                    std::string new_creation_date = year_str + "-12-31";

                    // Update rdf for each line that contains the following fields
                    rdf::update_rdf_line(result_id, "Creation-Date:", new_creation_date);
                    rdf::update_rdf_line(result_id, "File-URL:", new_url);
                    if ((std::stoi(page_range[1]) - std::stoi(page_range[0])) == 0) {
                        rdf::update_rdf_line(result_id, "Pages:", page_range[1]);
                    } else {
                        rdf::update_rdf_line(result_id, "Pages:", page_range[0] + " - " + page_range[1]);
                    }
                    rdf::update_rdf_line(result_id, "Year:", year_str);
                    rdf::update_rdf_line(result_id, "Volume:", vol_str);
                    rdf::update_rdf_line(result_id, "Issue:", iss_str);

                    rdf_updated = true;
                } else {
                    std::cout << "Skipped updating the rdf contents." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update all rdf contents for ID: " + result_id << std::endl;
                continue;
            }

            /* UPDATING HTML AND PDF TITLE PAGES FOR PUBLISHED PAPER */
            try {
                if (local_path_updated && rdf_updated) {
                    // Updates the stand-alone html title page (if it exists)
                    pdf::update_html(result_id, newVolumeNum, newIssueNum, page_range);
                    // Overwrites existing stand-alone pdf title page with updated html version
                    pdf::update_pdf(result_id);
                    // Removes the current title page from a published paper
                    pdf::remove_title_page(entry, result_id, temp_filename);
                    // Cats the title created during update_pdf() with the original published paper (minus old title) 
                    pdf::update_title_page(entry, result_id, temp_filename);
                } else {
                    std::cout << "Skipped updating with new title page." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
                std::cerr << "Failed to update title page for ID: " + result_id << std::endl;
                continue;
            }
        }
    }

    delete query;
    delete result;

    return 0;
}