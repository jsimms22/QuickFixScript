#include "sql_actions.h"

namespace sql_agent
{
    // Execute a SELECT query to retrieve a field for the entry from "tablepaper" table
    // For when only one condition is needed to find the field's value
    std::string retrieve_field(
        sql::Statement* query,
        sql::ResultSet* result,
        const std::string filename,
        const std::string field)
    {
        std::string output = "";
        result = query->executeQuery
            ("SELECT " + field + " FROM tablepaper WHERE Published_PDF_File LIKE '%" + filename + "' LIMIT 1; ");

        // Retrieve the row
        if (result->next()) { output = result->getString(1); }
        else { std::cout << "No rows found." << std::endl; }

        return output;
    }

    // Execute a SELECT query to retrieve a field for the entry from "tablepaper" table
    // For when two conditions is needed to find the field's value
    std::string retrieve_field(
        sql::Statement* query,
        sql::ResultSet* result, 
        const std::string paper_num,
        const std::string pub_dir,
        const std::string field)
    {
        std::string output = "";
        result = query->executeQuery
        ("SELECT " + field + " FROM tablepaper WHERE Published_PDF_File LIKE '%-P" + paper_num + ".pdf' AND Published_PDF_File LIKE '" + pub_dir + "%'; ");

        // Retrieve the row
        if (result->next()) { output = result->getString(1); }
        else { std::cout << "No rows found." << std::endl; }

        return output;
    }

    // Retrieves a field from the "tablepaperofarticles" table
    // Useful to retrieve a paper's abstract or title
    std::string retrieve_article_field(
        sql::Statement* query,
        sql::ResultSet* result,
        const std::string id,
        const std::string field)
    {
        std::string output = "";
        result = query->executeQuery
        ("SELECT " + field + " FROM tablepaperofarticles WHERE Article_ID LIKE '" + id + "' LIMIT 1; ");

        // Retrieve the row
        if (result->next()) { output = result->getString(1); }
        else { std::cout << "No rows found." << std::endl; }

        return output;
    }

    // Execute query to UPDATE field in "tablepaper" table with input string for the given id
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