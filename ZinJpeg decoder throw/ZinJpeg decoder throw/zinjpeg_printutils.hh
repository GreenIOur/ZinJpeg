#ifndef PRINT_UTILS_HH
#define PRINT_UTILS_HH
#include <iostream>

// In realta' c'e' definito anche errorocode_t...

/// ERROR CODES HELP:
// Negative values signal an error / marker.
// Positive values are success, and have meaning depending on the context.
typedef int errorcode_t;




inline void printbits(int size, unsigned long data) {
	for(int i = size-1; i >= 0; i--) {
		printf("%d",((data & (1<<i)) ? 1 : 0));
	}
}

#endif
