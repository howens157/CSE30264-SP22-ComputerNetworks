#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* rearrange_date(char* date, char result[]) {
	result[0] = date[4];
	result[1] = date[5];
	result[2] = date[0];
	result[3] = date[1];
	result[4] = date[2];
	result[5] = date[3];

	return result;
}

int compare_dates(char* start_date, char* end_date, char* date) {
	char rearranged_start_date[6];
	char rearranged_end_date[6];
	char rearranged_date[6];
	rearrange_date(start_date, rearranged_start_date);
	rearrange_date(end_date, rearranged_end_date);
	rearrange_date(date, rearranged_date);
	printf("%s\n%s\n%s\n", rearranged_start_date, rearranged_end_date, rearranged_date);
	int int_start_date = atoi(rearranged_start_date);
	int int_end_date = atoi(rearranged_end_date);
	int int_date = atoi(rearranged_date);
	return (int_date >= int_start_date && int_date <= int_end_date);
}

int main() {
	int result = compare_dates("032122", "032422", "032322");
	printf("%d\n", result);
	return 0;
}
