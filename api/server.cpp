#include "crow.h"
#include <mysql.h>
#include <iostream>
#include <string>
#include <cstdlib>

// ================================
// DATABASE CONNECTION
// ================================

MYSQL* connectDatabase()
{
    MYSQL* conn = mysql_init(NULL);

    if (conn == NULL)
    {
        std::cout << "mysql_init failed\n";
        return NULL;
    }

    // Read database credentials from environment variables
    const char* host = std::getenv("MYSQLHOST");
    const char* user = std::getenv("MYSQLUSER");
    const char* password = std::getenv("MYSQLPASSWORD");
    const char* database = std::getenv("MYSQLDATABASE");
    const char* portStr = std::getenv("MYSQLPORT");

    // Local development defaults
    if (!host) host = "localhost";
    if (!user) user = "root";
    if (!database) database = "delivery";

    unsigned int port = portStr
        ? static_cast<unsigned int>(std::stoi(portStr))
        : 3306;

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

    if (conn == NULL)
    {
        std::cout << "Database connection failed: "
                  << mysql_error(conn) << "\n";

        return NULL;
    }

    std::cout << "Database connected successfully!\n";

    return conn;
}


// ================================
// MAIN FUNCTION
// ================================

int main()
{
    MYSQL* conn = connectDatabase();

    if (conn == NULL)
    {
        return 1;
    }

    crow::SimpleApp app;


    // ============================================
    // API 1: GET ALL ORDERS
    // ============================================

    CROW_ROUTE(app, "/api/orders")
    .methods(crow::HTTPMethod::GET)
    ([conn]()
    {
        crow::json::wvalue response;
        crow::json::wvalue::list orders;

        const char* query =
            "SELECT id, dsp_id, driver_id, "
            "pickup_address, delivery_address, "
            "status, created_at, sla_deadline "
            "FROM orders "
            "ORDER BY id DESC";

        if (mysql_query(conn, query) != 0)
        {
            crow::response res(
                500,
                std::string("{\"error\":\"") +
                mysql_error(conn) +
                "\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_RES* result = mysql_store_result(conn);

        if (result == NULL)
        {
            crow::response res(
                500,
                "{\"error\":\"Could not retrieve orders\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)))
        {
            crow::json::wvalue order;

            order["id"] =
                row[0] ? std::stoi(row[0]) : 0;

            order["dsp_id"] =
                row[1] ? std::stoi(row[1]) : 0;

            order["driver_id"] =
                row[2] ? std::stoi(row[2]) : 0;

            order["pickup_address"] =
                row[3] ? row[3] : "";

            order["delivery_address"] =
                row[4] ? row[4] : "";

            order["status"] =
                row[5] ? row[5] : "";

            order["created_at"] =
                row[6] ? row[6] : "";

            order["sla_deadline"] =
                row[7] ? row[7] : "";

            orders.push_back(std::move(order));
        }

        mysql_free_result(result);

        response["orders"] = std::move(orders);

        crow::response res(response);

        // CORS
        res.set_header(
            "Access-Control-Allow-Origin",
            "*"
        );

        res.set_header(
            "Access-Control-Allow-Methods",
            "GET, POST, OPTIONS"
        );

        res.set_header(
            "Access-Control-Allow-Headers",
            "Content-Type"
        );

        return res;
    });


    // ============================================
    // API 2: DASHBOARD METRICS
    // ============================================

    CROW_ROUTE(app, "/api/metrics")
    .methods(crow::HTTPMethod::GET)
    ([conn]()
    {
        crow::json::wvalue response;

        const char* query =
            "SELECT "
            "COUNT(*) AS total, "
            "SUM(status = 'delivered') AS delivered, "
            "SUM(status != 'delivered') AS pending "
            "FROM orders";

        if (mysql_query(conn, query) != 0)
        {
            crow::response res(
                500,
                std::string("{\"error\":\"") +
                mysql_error(conn) +
                "\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_RES* result = mysql_store_result(conn);

        if (result == NULL)
        {
            crow::response res(
                500,
                "{\"error\":\"Could not retrieve metrics\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_ROW row = mysql_fetch_row(result);

        response["totalOrders"] =
            row && row[0] ? std::stoi(row[0]) : 0;

        response["delivered"] =
            row && row[1] ? std::stoi(row[1]) : 0;

        response["pending"] =
            row && row[2] ? std::stoi(row[2]) : 0;

        mysql_free_result(result);

        crow::response res(response);

        res.set_header(
            "Access-Control-Allow-Origin",
            "*"
        );

        return res;
    });


    // ============================================
    // API 3: ACTIVE DRIVERS
    // ============================================

    CROW_ROUTE(app, "/api/active-drivers")
    .methods(crow::HTTPMethod::GET)
    ([conn]()
    {
        crow::json::wvalue response;

        const char* query =
            "SELECT COUNT(DISTINCT driver_id) "
            "FROM orders "
            "WHERE driver_id IS NOT NULL "
            "AND status != 'delivered'";

        if (mysql_query(conn, query) != 0)
        {
            crow::response res(
                500,
                std::string("{\"error\":\"") +
                mysql_error(conn) +
                "\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }

        MYSQL_RES* result = mysql_store_result(conn);

        MYSQL_ROW row = mysql_fetch_row(result);

        response["activeDrivers"] =
            row && row[0] ? std::stoi(row[0]) : 0;

        mysql_free_result(result);

        crow::response res(response);

        res.set_header(
            "Access-Control-Allow-Origin",
            "*"
        );

        return res;
    });


    // ============================================
    // API 4: SLA VIOLATIONS
    // ============================================

    CROW_ROUTE(app, "/api/sla-violations")
    .methods(crow::HTTPMethod::GET)
    ([conn]()
    {
        crow::json::wvalue response;
        crow::json::wvalue::list violations;

        const char* query =
            "SELECT id, status, sla_deadline "
            "FROM orders "
            "WHERE sla_deadline < NOW() "
            "AND status != 'delivered'";

        if (mysql_query(conn, query) != 0)
        {
            crow::response res(
                500,
                std::string("{\"error\":\"") +
                mysql_error(conn) +
                "\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_RES* result = mysql_store_result(conn);

        if (result == NULL)
        {
            crow::response res(
                500,
                "{\"error\":\"Could not retrieve SLA violations\"}"
            );

            res.set_header(
                "Access-Control-Allow-Origin",
                "*"
            );

            return res;
        }


        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)))
        {
            crow::json::wvalue violation;

            violation["orderId"] =
                row[0] ? std::stoi(row[0]) : 0;

            violation["status"] =
                row[1] ? row[1] : "";

            violation["slaDeadline"] =
                row[2] ? row[2] : "";

            violations.push_back(std::move(violation));
        }

        mysql_free_result(result);

        response["violations"] =
            std::move(violations);

        crow::response res(response);

        res.set_header(
            "Access-Control-Allow-Origin",
            "*"
        );

        return res;
    });


    // ============================================
    // START SERVER
    // ============================================

    std::cout << "\n====================================\n";
    std::cout << " DELIVERY MANAGEMENT API SERVER\n";
    std::cout << "====================================\n";

    std::cout << "\nAPI running at:\n";
    std::cout << "http://127.0.0.1:18080\n";

   const char* portEnv = std::getenv("PORT");

int serverPort = portEnv
    ? std::stoi(portEnv)
    : 18080;

std::cout << "Server running on port: "
          << serverPort << "\n";

app.port(serverPort)
   .multithreaded()
   .run();


    mysql_close(conn);

    return 0;
}