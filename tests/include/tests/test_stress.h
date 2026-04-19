#pragma once
#include "stdio.h"

#define TEST_DURATION (30 * 60 * 1000) //trajanje testa - 30 minuta
#define SYSTEM_STATISTICS_LOG_INTERVAL (30 * 1000) //svakih koliko logiramo stanje - 30 sekundi

void stress_run_test(void);