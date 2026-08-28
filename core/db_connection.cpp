#include "db_connection.h"
#include <iostream>

MYSQL* getConnection() {

    MYSQL* conn = mysql_init(NULL);

    if (conn == NULL) {
        std::cerr << "mysql_init failed!" << std::endl;
        return NULL;
    }

    conn = mysql_real_connect(
        conn,
        "localhost",
        "root",
        "ANUHYA",
        "delivery",
        3306,
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