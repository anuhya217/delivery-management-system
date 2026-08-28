#ifndef ORDER_SERVICES_H
#define ORDER_SERVICES_H


#include <mysql.h>
void checkSLAViolations(MYSQL* conn);
void showSystemMetrics(MYSQL* conn);

void createOrder(MYSQL* conn);
void listOrders(MYSQL* conn);
void assignDriverToOrder(MYSQL* conn);
void updateOrderStatus(MYSQL* conn);
void viewOrderStatusHistory(MYSQL* conn);

#endif