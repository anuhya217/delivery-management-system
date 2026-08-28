#ifndef DRIVER_SERVICES_H
#define DRIVER_SERVICES_H

#include <mysql.h>

void createDriver(MYSQL* conn);
void listDrivers(MYSQL* conn);
void updateDriverStatus(MYSQL* conn);

#endif