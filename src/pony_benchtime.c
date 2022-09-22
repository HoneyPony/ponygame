#include "pony_benchtime.h"

BTime bt_start() {
	return (BTime){clock()};
}

double bt_passed_ms(BTime start) {
	clock_t elapsed = clock() - start.time;

	double sec = elapsed / (double)CLOCKS_PER_SEC;
	return (sec * 1000.0);
}