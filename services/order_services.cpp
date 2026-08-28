#include "order_services.h"

#include <iostream>
#include <string>

using namespace std;


// ================= CREATE ORDER =================

void createOrder(MYSQL* conn)
{
    int dspId;
    string pickupAddress;
    string deliveryAddress;
    string slaDeadline;

    cout << "\nEnter DSP ID: ";
    cin >> dspId;

    cout << "Enter Pickup Address: ";
    getline(cin >> ws, pickupAddress);

    cout << "Enter Delivery Address: ";
    getline(cin, deliveryAddress);

    cout << "Enter SLA Deadline (YYYY-MM-DD HH:MM:SS): ";
    getline(cin, slaDeadline);

    string query =
        "INSERT INTO orders "
        "(dsp_id, pickup_address, delivery_address, sla_deadline) "
        "VALUES (" +
        to_string(dspId) + ", '" +
        pickupAddress + "', '" +
        deliveryAddress + "', '" +
        slaDeadline + "')";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error creating order: "
             << mysql_error(conn) << endl;
        return;
    }

    cout << "Order created successfully!" << endl;
}


// ================= LIST ORDERS =================

void listOrders(MYSQL* conn)
{
    string query =
        "SELECT id, dsp_id, driver_id, pickup_address, "
        "delivery_address, status, created_at, sla_deadline "
        "FROM orders";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error retrieving orders: "
             << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL)
    {
        cout << "No orders found." << endl;
        return;
    }

    MYSQL_ROW row;

    cout << "\n========== ORDER LIST ==========\n";

    while ((row = mysql_fetch_row(result)))
    {
        cout << "\nOrder ID: " << row[0]
             << "\nDSP ID: " << row[1]
             << "\nDriver ID: " << (row[2] ? row[2] : "Not Assigned")
             << "\nPickup: " << row[3]
             << "\nDelivery: " << row[4]
             << "\nStatus: " << row[5]
             << "\nCreated: " << row[6]
             << "\nSLA Deadline: " << (row[7] ? row[7] : "Not Set")
             << "\n-----------------------------\n";
    }

    mysql_free_result(result);
}


// ================= ASSIGN DRIVER =================

void assignDriverToOrder(MYSQL* conn)
{
    int orderId;
    int driverId;

    cout << "\nEnter Order ID: ";
    cin >> orderId;

    cout << "Enter Driver ID: ";
    cin >> driverId;

    string query =
        "UPDATE orders SET driver_id = " +
        to_string(driverId) +
        ", status = 'assigned' "
        "WHERE id = " +
        to_string(orderId);

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error assigning driver: "
             << mysql_error(conn) << endl;
        return;
    }

    if (mysql_affected_rows(conn) == 0)
    {
        cout << "Order not found!" << endl;
        return;
    }

    cout << "Driver assigned successfully!" << endl;
}


// ================= UPDATE ORDER STATUS =================

void updateOrderStatus(MYSQL* conn)
{
    int orderId;
    string newStatus;

    cout << "\nEnter Order ID: ";
    cin >> orderId;

    cout << "Enter New Status "
         << "(picked_up/in_transit/delivered/failed): ";

    getline(cin >> ws, newStatus);


    // STEP 1: Get current status
    string selectQuery =
        "SELECT status FROM orders WHERE id = " +
        to_string(orderId);

    if (mysql_query(conn, selectQuery.c_str()))
    {
        cout << "Error: " << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL || mysql_num_rows(result) == 0)
    {
        cout << "Order not found!" << endl;

        if (result != NULL)
            mysql_free_result(result);

        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    string oldStatus = row[0];

    mysql_free_result(result);


    // STEP 2: Update order status
    string updateQuery =
        "UPDATE orders SET status = '" +
        newStatus +
        "' WHERE id = " +
        to_string(orderId);

    if (mysql_query(conn, updateQuery.c_str()))
    {
        cout << "Error updating order: "
             << mysql_error(conn) << endl;

        return;
    }


    // STEP 3: Insert audit log
    string logQuery =
        "INSERT INTO delivery_status_log "
        "(order_id, old_status, new_status) VALUES (" +
        to_string(orderId) + ", '" +
        oldStatus + "', '" +
        newStatus + "')";

    if (mysql_query(conn, logQuery.c_str()))
    {
        cout << "Status updated, but logging failed: "
             << mysql_error(conn) << endl;

        return;
    }

    cout << "Order status updated successfully!" << endl;
    cout << "Status change logged successfully!" << endl;
}
void viewOrderStatusHistory(MYSQL* conn)
{
    int orderId;

    cout << "\nEnter Order ID: ";
    cin >> orderId;

    string query =
        "SELECT old_status, new_status, changed_at "
        "FROM delivery_status_log "
        "WHERE order_id = " +
        to_string(orderId) +
        " ORDER BY changed_at ASC";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error retrieving status history: "
             << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL)
    {
        cout << "Error retrieving status history." << endl;
        return;
    }

    MYSQL_ROW row;

    cout << "\n===== ORDER STATUS HISTORY =====\n";

    bool found = false;

    while ((row = mysql_fetch_row(result)))
    {
        found = true;

        cout << "Old Status: " << row[0]
             << " -> New Status: " << row[1]
             << " | Time: " << row[2]
             << endl;
    }

    if (!found)
    {
        cout << "No status history found for this order." << endl;
    }

    mysql_free_result(result);
}
void checkSLAViolations(MYSQL* conn)
{
    string query =
        "SELECT id, dsp_id, driver_id, status, sla_deadline "
        "FROM orders "
        "WHERE sla_deadline IS NOT NULL "
        "AND sla_deadline < NOW() "
        "AND status != 'delivered'";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error checking SLA violations: "
             << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL)
    {
        cout << "Error retrieving SLA data." << endl;
        return;
    }

    MYSQL_ROW row;

    bool found = false;

    cout << "\n===== SLA VIOLATIONS =====\n";

    while ((row = mysql_fetch_row(result)))
    {
        found = true;

        cout << "\n🚨 SLA VIOLATION"
             << "\nOrder ID: " << row[0]
             << "\nDSP ID: " << row[1]
             << "\nDriver ID: " << (row[2] ? row[2] : "Not Assigned")
             << "\nStatus: " << row[3]
             << "\nSLA Deadline: " << row[4]
             << "\n-------------------------\n";
    }

    if (!found)
    {
        cout << "No SLA violations found. System healthy!\n";
    }

    mysql_free_result(result);
}
void showSystemMetrics(MYSQL* conn)
{
    string query =
        "SELECT "
        "COUNT(*) AS total_orders, "
        "SUM(status = 'delivered') AS delivered, "
        "SUM(status = 'failed') AS failed, "
        "SUM(status = 'in_transit') AS in_transit, "
        "SUM(sla_deadline < NOW() AND status != 'delivered') "
        "AS sla_violations "
        "FROM orders";

    if (mysql_query(conn, query.c_str()))
    {
        cout << "Error retrieving metrics: "
             << mysql_error(conn) << endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL)
    {
        cout << "Error retrieving metrics." << endl;
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    cout << "\n===== SYSTEM METRICS DASHBOARD =====\n";

    cout << "Total Orders     : " << (row[0] ? row[0] : "0") << endl;
    cout << "Delivered        : " << (row[1] ? row[1] : "0") << endl;
    cout << "Failed           : " << (row[2] ? row[2] : "0") << endl;
    cout << "In Transit       : " << (row[3] ? row[3] : "0") << endl;
    cout << "SLA Violations   : " << (row[4] ? row[4] : "0") << endl;

    cout << "====================================\n";

    mysql_free_result(result);
}