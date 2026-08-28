#include "db_connection.h"
#include <iostream>

int main() {

    MYSQL* conn = getConnection();

    if (conn == NULL) {
        std::cout << "Database connection failed!" << std::endl;
        return 1;
    }

    std::cout << "Database connected successfully!" << std::endl;

    closeConnection(conn);

    return 0;
}