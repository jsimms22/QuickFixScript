#include "sql_agent.h"

namespace sql_agent
{
	MySQL_Interface::MySQL_Interface()
	{
		this->m_server = sql_agent::ServerInfo{sql_agent::Protocol::TCP, "127.0.0.1", "3306"};
		this->m_conn = nullptr;
		this->m_sql_driver = nullptr;
	}

	void MySQL_Interface::set_server(const Protocol _ev, const std::string _ip, const std::string _port)
	{
		this->m_server = sql_agent::ServerInfo{_ev, _ip, _port};
	}

	void MySQL_Interface::set_schema(const std::string _str) { this->m_db_schema = _str; }

	void MySQL_Interface::set_user(const std::string _username, const std::string _password)
	{
		this->m_user = sql_agent::UserCredentials{_username, _password};
	}

	void MySQL_Interface::set_driver()
	{
		if (this->m_sql_driver == nullptr) {
			this->m_sql_driver = sql::mysql::get_mysql_driver_instance();
		} else {
			std::cerr << "mysql driver instance has already been initialized" << std::endl;
		}
	}

	void MySQL_Interface::set_connection() 
	{
		try {
			if (m_server.transport == sql_agent::Protocol::TCP) {
				this->m_conn = m_sql_driver->connect(
					"tcp://" + m_server.ip + ":" + m_server.port, 
					m_user.username, 
					m_user.password);
				std::cout << "Setting connection to tcp://" + m_server.ip + ":" + m_server.port 
					+ "\nwith credentials: " + m_user.username + " " + m_user.password << std::endl;
			} else if (m_server.transport == sql_agent::Protocol::UDP) {
				std::cerr << "UDP undefined for now." << std::endl;
				throw;
			} else {
				std::cerr << "Unknown transport protocol." << std::endl;
				throw;
			}
			this->m_conn->setSchema(m_db_schema);
		} catch (sql::SQLException& e) {
			// Query error
			std::cerr << "Query error: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown error occurred while attempting to set connection to the database." << std::endl;
		}
	}
	
	sql::Connection* MySQL_Interface::get_connection() { return this->m_conn; }

	MySQL_Interface::~MySQL_Interface() {
		// Clean up the MySQL driver if it is allocated
		if (m_sql_driver) {
			delete m_sql_driver;
		}
		// Clean up the MySQL connection if it is allocated
		if (m_conn) {
			delete m_conn;
		}
	}
}