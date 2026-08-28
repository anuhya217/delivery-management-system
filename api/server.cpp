#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <mysql/mysql.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

MYSQL* connectDatabase()
{
    MYSQL* conn = mysql_init(nullptr);

    if (conn == nullptr)
    {
        std::cerr << "mysql_init failed\n";
        return nullptr;
    }

    // Railway environment variables
    const char* host = std::getenv("MYSQLHOST");
    const char* user = std::getenv("MYSQLUSER");
    const char* password = std::getenv("MYSQLPASSWORD");
    const char* database = std::getenv("MYSQLDATABASE");
    const char* portEnv = std::getenv("MYSQLPORT");

    // Local fallback values
    if (host == nullptr) host = "localhost";
    if (user == nullptr) user = "root";
    if (password == nullptr) password = "ANUHYA";
    if (database == nullptr) database = "delivery";

    unsigned int port = 3306;

    if (portEnv != nullptr)
    {
        port = std::stoi(portEnv);
    }

    if (mysql_real_connect(
            conn,
            host,
            user,
            password,
            database,
            port,
            nullptr,
            0) == nullptr)
    {
        std::cerr << "Database connection failed: "
                  << mysql_error(conn) << "\n";

        mysql_close(conn);
        return nullptr;
    }

    std::cout << "Database connected successfully!\n";

    return conn;
}


// Escape JSON strings
std::string escapeJson(const char* value)
{
    if (value == nullptr)
        return "";

    std::string input(value);
    std::string output;

    for (char c : input)
    {
        switch (c)
        {
            case '"':
                output += "\\\"";
                break;

            case '\\':
                output += "\\\\";
                break;

            case '\n':
                output += "\\n";
                break;

            case '\r':
                output += "\\r";
                break;

            case '\t':
                output += "\\t";
                break;

            default:
                output += c;
        }
    }

    return output;
}


// GET /api/orders
std::string getOrders(MYSQL* conn)
{
    const char* query =
        "SELECT id, dsp_id, driver_id, "
        "pickup_address, delivery_address, status, "
        "created_at, sla_deadline "
        "FROM orders ORDER BY id DESC";

    if (mysql_query(conn, query) != 0)
    {
        std::string error = mysql_error(conn);

        return "{\"error\":\"" +
               escapeJson(error.c_str()) +
               "\"}";
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == nullptr)
    {
        return "{\"error\":\"Could not retrieve orders\"}";
    }

    std::stringstream json;

    json << "{\"orders\":[";

    MYSQL_ROW row;

    bool first = true;

    while ((row = mysql_fetch_row(result)))
    {
        if (!first)
        {
            json << ",";
        }

        first = false;

        json << "{";

        json << "\"id\":"
             << (row[0] ? row[0] : "0") << ",";

        json << "\"dsp_id\":"
             << (row[1] ? row[1] : "0") << ",";

        if (row[2] != nullptr)
        {
            json << "\"driver_id\":"
                 << row[2] << ",";
        }
        else
        {
            json << "\"driver_id\":null,";
        }

        json << "\"pickup_address\":\""
             << escapeJson(row[3]) << "\",";

        json << "\"delivery_address\":\""
             << escapeJson(row[4]) << "\",";

        json << "\"status\":\""
             << escapeJson(row[5]) << "\",";

        json << "\"created_at\":\""
             << escapeJson(row[6]) << "\",";

        json << "\"sla_deadline\":\""
             << escapeJson(row[7]) << "\"";

        json << "}";
    }

    json << "]}";

    mysql_free_result(result);

    return json.str();
}


// GET /api/metrics
std::string getMetrics(MYSQL* conn)
{
    const char* query =
        "SELECT "
        "COUNT(*) AS total, "
        "COALESCE(SUM(status = 'delivered'), 0) AS delivered, "
        "COALESCE(SUM(status = 'failed'), 0) AS failed, "
        "COALESCE(SUM(status != 'delivered'), 0) AS pending "
        "FROM orders";

    if (mysql_query(conn, query) != 0)
    {
        std::string error = mysql_error(conn);

        return "{\"error\":\"" +
               escapeJson(error.c_str()) +
               "\"}";
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == nullptr)
    {
        return "{\"error\":\"Could not retrieve metrics\"}";
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    std::stringstream json;

    json << "{";

    json << "\"totalOrders\":"
         << (row && row[0] ? row[0] : "0") << ",";

    json << "\"delivered\":"
         << (row && row[1] ? row[1] : "0") << ",";

    json << "\"failed\":"
         << (row && row[2] ? row[2] : "0") << ",";

    json << "\"pending\":"
         << (row && row[3] ? row[3] : "0");

    json << "}";

    mysql_free_result(result);

    return json.str();
}


// GET /api/sla-violations
std::string getSLAViolations(MYSQL* conn)
{
    const char* query =
        "SELECT id, status, sla_deadline "
        "FROM orders "
        "WHERE sla_deadline < NOW() "
        "AND status != 'delivered'";

    if (mysql_query(conn, query) != 0)
    {
        std::string error = mysql_error(conn);

        return "{\"error\":\"" +
               escapeJson(error.c_str()) +
               "\"}";
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == nullptr)
    {
        return "{\"error\":\"Could not retrieve SLA violations\"}";
    }

    std::stringstream json;

    json << "{\"violations\":[";

    MYSQL_ROW row;

    bool first = true;

    while ((row = mysql_fetch_row(result)))
    {
        if (!first)
        {
            json << ",";
        }

        first = false;

        json << "{";

        json << "\"orderId\":"
             << (row[0] ? row[0] : "0") << ",";

        json << "\"status\":\""
             << escapeJson(row[1]) << "\",";

        json << "\"slaDeadline\":\""
             << escapeJson(row[2]) << "\"";

        json << "}";
    }

    json << "]}";

    mysql_free_result(result);

    return json.str();
}


// Send HTTP response
void sendResponse(
    int clientSocket,
    const std::string& body,
    int statusCode = 200
)
{
    std::string statusText =
        statusCode == 200 ? "OK" :
        statusCode == 404 ? "Not Found" :
        "Internal Server Error";

    std::string response =
        "HTTP/1.1 " +
        std::to_string(statusCode) +
        " " +
        statusText +
        "\r\n";

    response +=
        "Content-Type: application/json\r\n";

    // CORS for frontend
    response +=
        "Access-Control-Allow-Origin: *\r\n";

    response +=
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n";

    response +=
        "Access-Control-Allow-Headers: Content-Type\r\n";

    response +=
        "Content-Length: " +
        std::to_string(body.length()) +
        "\r\n";

    response +=
        "Connection: close\r\n\r\n";

    response += body;

    send(
        clientSocket,
        response.c_str(),
        response.length(),
        0
    );
}


int main()
{
    MYSQL* conn = connectDatabase();

    if (conn == nullptr)
    {
        return 1;
    }


    // Railway provides PORT
    const char* portEnv = std::getenv("PORT");

    int port = 18080;

    if (portEnv != nullptr)
    {
        port = std::stoi(portEnv);
    }


    int serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (serverSocket < 0)
    {
        std::cerr << "Could not create socket\n";

        mysql_close(conn);

        return 1;
    }


    int opt = 1;

    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );


    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;

    // Railway needs 0.0.0.0
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    serverAddress.sin_port =
        htons(port);


    if (bind(
            serverSocket,
            (sockaddr*)&serverAddress,
            sizeof(serverAddress)
        ) < 0)
    {
        std::cerr << "Bind failed\n";

        close(serverSocket);

        mysql_close(conn);

        return 1;
    }


    if (listen(serverSocket, 10) < 0)
    {
        std::cerr << "Listen failed\n";

        close(serverSocket);

        mysql_close(conn);

        return 1;
    }


    std::cout << "\n=================================\n";
    std::cout << "Delivery Management API Running\n";
    std::cout << "Port: " << port << "\n";
    std::cout << "=================================\n";

    std::cout << "Endpoints:\n";
    std::cout << "/api/orders\n";
    std::cout << "/api/metrics\n";
    std::cout << "/api/sla-violations\n";


    while (true)
    {
        sockaddr_in clientAddress{};

        socklen_t clientLength =
            sizeof(clientAddress);


        int clientSocket = accept(
            serverSocket,
            (sockaddr*)&clientAddress,
            &clientLength
        );


        if (clientSocket < 0)
        {
            continue;
        }


        char buffer[4096] = {0};


        int bytesReceived =
            recv(
                clientSocket,
                buffer,
                sizeof(buffer) - 1,
                0
            );


        if (bytesReceived <= 0)
        {
            close(clientSocket);
            continue;
        }


        std::string request(buffer);


        std::string body;
        int statusCode = 200;


        // GET /api/orders
        if (
            request.find(
                "GET /api/orders "
            ) != std::string::npos
        )
        {
            body = getOrders(conn);
        }


        // GET /api/metrics
        else if (
            request.find(
                "GET /api/metrics "
            ) != std::string::npos
        )
        {
            body = getMetrics(conn);
        }


        // GET /api/sla-violations
        else if (
            request.find(
                "GET /api/sla-violations "
            ) != std::string::npos
        )
        {
            body = getSLAViolations(conn);
        }


        // Health check
        else if (
            request.find(
                "GET / "
            ) != std::string::npos
        )
        {
            body =
                "{\"status\":\"Delivery API is running\"}";
        }


        else
        {
            body =
                "{\"error\":\"Endpoint not found\"}";

            statusCode = 404;
        }


        sendResponse(
            clientSocket,
            body,
            statusCode
        );


        close(clientSocket);
    }


    close(serverSocket);

    mysql_close(conn);

    return 0;
}