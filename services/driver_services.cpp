#include "driver_services.h"

#include <iostream>
#include <string>

using namespace std;

void createDriver(MYSQL* conn)
{
    int dspId;
    string name;
    string status;

    cout << "\nEnter DSP ID: ";
    cin >> dspId;

    cout << "Enter Driver Name: ";
    getline(cin >> ws, name);

    cout << "Enter Status (available/busy/offline): ";
    getline(cin, status);

    string query =
        "INSERT INTO drivers (dsp_id, name, status) VALUES (" +
        to_string(dspId) + ", '" +
        name + "', '" +
        status + "')";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error creating driver: "
             << mysql_error(conn) << endl;
        return;
    }

    cout << "Driver created successfully!" << endl;
}

void listDrivers(MYSQL* conn)
{
    string query =
        "SELECT id, dsp_id, name, status, created_at FROM drivers";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error: "
             << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;

    cout << "\n------ DRIVER LIST ------\n";

    while ((row = mysql_fetch_row(result)))
    {
        cout << "ID: " << row[0]
             << " | DSP ID: " << row[1]
             << " | Name: " << row[2]
             << " | Status: " << row[3]
             << endl;
    }

    mysql_free_result(result);
}

void updateDriverStatus(MYSQL* conn)
{
    int driverId;
    string status;

    cout << "\nEnter Driver ID: ";
    cin >> driverId;

    cout << "Enter New Status: ";
    getline(cin >> ws, status);

    string query =
        "UPDATE drivers SET status = '" +
        status +
        "' WHERE id = " +
        to_string(driverId);

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error updating driver: "
             << mysql_error(conn) << endl;
        return;
    }

    cout << "Driver status updated successfully!" << endl;
}