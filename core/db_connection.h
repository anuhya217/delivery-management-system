#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <mysql.h>

MYSQL* getConnection();

void closeConnection(MYSQL* conn);

#endif