#include "database.h"
#include <iostream>

using namespace std;

MYSQL* connectDatabase()
{
    MYSQL* conn = mysql_init(NULL);

    if (conn == NULL)
    {
        cout << "mysql_init failed!" << endl;
        return NULL;
    }

    if (mysql_real_connect(
            conn,
            "localhost",
            "root",
            "ANUHYA",
            "delivery",
            3306,
            NULL,
            0) == NULL)
    {
        cout << "Database connection failed: "
             << mysql_error(conn) << endl;

        mysql_close(conn);
        return NULL;
    }

    cout << "Database connected successfully!" << endl;

    return conn;
}