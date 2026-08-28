#include "dsp_services.h"
#include <iostream>
#include <string>

using namespace std;

void createDSP(MYSQL* conn)
{
    string name, region;

    cout << "\nEnter DSP Name: ";
    cin.ignore();
    getline(cin, name);

    cout << "Enter Region: ";
    getline(cin, region);

    string query =
        "INSERT INTO dsps (name, region) VALUES ('" +
        name + "', '" + region + "')";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
    }
    else
    {
        cout << "DSP created successfully!\n";
    }
}

void listDSPs(MYSQL* conn)
{
    string query = "SELECT * FROM dsps";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    MYSQL_ROW row;

    cout << "\n--- DSP LIST ---\n";

    while ((row = mysql_fetch_row(result)))
    {
        cout << "ID: " << row[0]
             << " | Name: " << row[1]
             << " | Region: " << row[2]
             << endl;
    }

    mysql_free_result(result);
}