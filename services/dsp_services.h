#ifndef DSP_SERVICES_H
#define DSP_SERVICES_H

#include <mysql.h>

void createDSP(MYSQL* conn);
void listDSPs(MYSQL* conn);

#endif