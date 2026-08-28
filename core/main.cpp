#include <iostream>
#include "database.h"
#include "dsp_services.h"
#include "driver_services.h"
#include "order_services.h"

using namespace std;

int main()
{
    MYSQL* conn = connectDatabase();

    if (conn == NULL)
    {
        return 1;
    }

    int choice;

    while (true)
    {
        cout << "\n===== DELIVERY MANAGEMENT SYSTEM =====\n";
        cout << "1. Create DSP\n";
        cout << "2. List DSPs\n";
        cout << "3. Add Driver\n";
        cout << "4. List Drivers\n";
        cout << "5. Update Driver Status\n";
        cout << "6. Create Order\n";
cout << "7. List Orders\n";
cout << "8. Assign Driver to Order\n";
cout << "9. Update Order Status\n";
cout << "10. View Order Status History\n";
cout << "11. Check SLA Violations\n";
cout << "12. System Metrics Dashboard\n";
        cout << "0. Exit\n";

        cout << "\nEnter choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                createDSP(conn);
                break;

            case 2:
                listDSPs(conn);
                break;

            case 3:
                createDriver(conn);
                break;

            case 4:
                listDrivers(conn);
                break;

            case 5:
                updateDriverStatus(conn);
                break;
                case 6:
    createOrder(conn);
    break;

case 7:
    listOrders(conn);
    break;

case 8:
    assignDriverToOrder(conn);
    break;

case 9:
    updateOrderStatus(conn);
    break;
    case 10:
    viewOrderStatusHistory(conn);
    break;
    case 11:
    checkSLAViolations(conn);
    break;

case 12:
    showSystemMetrics(conn);
    break;

            case 0:
                mysql_close(conn);
                cout << "Goodbye!\n";
                return 0;

            default:
                cout << "Invalid choice!\n";
        }
    }

    return 0;
}