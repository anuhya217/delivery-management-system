#include "db_connection.h"

#include <iostream>
#include <cstdlib>

MYSQL* getConnection() {

    // Railway environment variables
    const char* host = std::getenv("MYSQLHOST");
    const char* user = std::getenv("MYSQLUSER");
    const char* password = std::getenv("MYSQLPASSWORD");
    const char* database = std::getenv("MYSQL_DATABASE");
    const char* portStr = std::getenv("MYSQLPORT");

    // Check variables
    if (!host || !user || !password || !database || !portStr) {
        std::cerr << "Database environment variables are missing!" << std::endl;
        return NULL;
    }

    unsigned int port = std::stoi(portStr);

    MYSQL* conn = mysql_init(NULL);

    if (conn == NULL) {
        std::cerr << "mysql_init failed!" << std::endl;
        return NULL;
    }

    // Connect to Railway MySQL
    if (mysql_real_connect(
            conn,
            host,
            user,
            password,
            database,
            port,
            NULL,
            0
        ) == NULL) {

        std::cerr << "Database connection failed: "
                  << mysql_error(conn)
                  << std::endl;

        mysql_close(conn);
        return NULL;
    }

    std::cout << "Database connected successfully!" << std::endl;

    return conn;
}

void closeConnection(MYSQL* conn) {

    if (conn != NULL) {
        mysql_close(conn);
    }
}