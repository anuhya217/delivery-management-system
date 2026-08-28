#include "db_connection.h"
#include <iostream>
#include <cstdlib>

MYSQL* getConnection() {

    MYSQL* conn = mysql_init(NULL);

    if (conn == NULL) {
        std::cerr << "mysql_init failed!" << std::endl;
        return NULL;
    }

    const char* host = std::getenv("MYSQL_HOST");
    const char* user = std::getenv("MYSQL_USER");
    const char* password = std::getenv("MYSQL_PASSWORD");
    const char* database = std::getenv("MYSQL_DATABASE");
    const char* portStr = std::getenv("MYSQL_PORT");

    if (!host || !user || !password || !database || !portStr) {
        std::cerr << "Database environment variables are missing!" << std::endl;
        return NULL;
    }

    int port = std::atoi(portStr);

    conn = mysql_real_connect(
        conn,
        host,
        user,
        password,
        database,
        port,
        NULL,
        0
    );

    if (conn == NULL) {
        std::cerr << "Connection failed: "
                  << mysql_error(conn)
                  << std::endl;

        return NULL;
    }

    return conn;
}

void closeConnection(MYSQL* conn) {
    if (conn != NULL) {
        mysql_close(conn);
    }
}