#include "sql_actions.h"

namespace sql_agent
{
    // Execute a SELECT query to retrieve ID field for the entry
    std::string retrieve_field(
        sql::Statement* query, 
        sql::ResultSet* result, 
        std::string filename, 
        std::string field)
    {
        std::string output = "";
        result = query->executeQuery
            ("SELECT " + field + " FROM tablepaper WHERE Published_PDF_File LIKE '%" + filename + "' LIMIT 1; ");

        // Retrieve the row
        if (result->next()) { output = result->getString(1); }
        else { std::cout << "No rows found." << std::endl; }

        return output;
    }

    // Execute query to UPDATE field with input_str for the given id
    void update_field_by_ID(
        sql::Statement* query, 
        const std::string id, 
        const std::string field, 
        const std::string input_str)
    {
        query->executeUpdate
            ("UPDATE tablepaper SET " + field + " = '" + input_str + "' WHERE id = '" + id + "';");
    }
}