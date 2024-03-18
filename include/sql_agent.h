#pragma once

// mysql_connector/c++ headers
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
// standard library headers
#include <memory>

namespace sql_agent
{
	enum class Protocol { TCP, UDP };

	struct ServerInfo { Protocol transport; std::string ip; std::string port; };

	struct UserCredentials { std::string username; std::string password; };

	class MySQL_Interface
	{
	public:
		MySQL_Interface();

		void set_server(Protocol, std::string, std::string);
		void set_schema(std::string);
		void set_user(std::string, std::string);
		void set_driver();
		void set_connection();

		sql::Connection* get_connection();

		~MySQL_Interface();
	private:
		ServerInfo m_server;
		std::string m_db_schema;
		UserCredentials m_user;

		sql::mysql::MySQL_Driver* m_sql_driver;
		sql::Connection* m_conn;
	};
}