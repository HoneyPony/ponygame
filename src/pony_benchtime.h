#pragma once

#include <time.h>

typedef struct {
	clock_t time;
} BTime;

BTime bt_start();
double bt_passed_ms(BTime start);