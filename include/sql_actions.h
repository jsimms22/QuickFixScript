#pragma once

#include "sql_agent.h"

namespace sql_agent
{
	// Execute a SELECT query to retrieve a field for the entry from "tablepaper" table
	// For when only one condition is needed to find the field's value
	std::string retrieve_field(sql::Statement*, sql::ResultSet*, const std::string, const std::string);

	// Execute a SELECT query to retrieve a field for the entry from "tablepaper" table
	// For when two conditions is needed to find the field's value
	std::string retrieve_field(sql::Statement*, sql::ResultSet*, const std::string, const std::string, const std::string);

	// Retrieves a field from the "tablepaperofarticles" table
	// Useful to retrieve a paper's abstract or title
	std::string retrieve_article_field(sql::Statement*, sql::ResultSet*, const std::string, const std::string);

	// Execute query to UPDATE field in "tablepaper" table with input string for the given id
	void update_field_by_ID(sql::Statement*, const std::string, const std::string, const std::string);
}