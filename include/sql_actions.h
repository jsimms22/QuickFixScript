#pragma once

#include "sql_agent.h"

namespace sql_agent
{
	std::string retrieve_field(sql::Statement*, sql::ResultSet*, std::string, std::string);

	void update_field_by_ID(sql::Statement*, const std::string, const std::string, const std::string);
}