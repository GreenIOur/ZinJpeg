static unsigned overhead_data = 0;		// bytes
static unsigned no_overhead_data = 0;		// bits

	
void OverheadData(unsigned length) {
	overhead_data += length;
}

void NoOverheadData(unsigned length) {
	no_overhead_data += length;
}

unsigned GetOverheadStats() {
	return overhead_data*8;
}

unsigned GetNoOverheadStats() {
	return no_overhead_data;
}

void ResetStats() {
	no_overhead_data = 0;
	overhead_data = 0;
}
