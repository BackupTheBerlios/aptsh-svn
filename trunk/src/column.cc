#include <string>
#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

#include "column.h"

column_display::column_display(int columns, char separator) : cols(columns), sep(separator)
{
	if (columns > 6) {
		fprintf(stderr, "Maximum number of columns is 6!\n");
	}
}

void column_display::add (char* text, int column)
{
	if (column > cols) {
		fprintf(stderr, "Such column does not exist!\n");
		return;
	}

	flesh[column].push_back(string(text));
}

void column_display::dump()
{
	int biggest[5] = {0, 0, 0, 0, 0};
	for (int i = 0; i < cols; i++) {
		for (unsigned long j = 0; j < flesh[i].size(); j++) {
			if (flesh[i][j].size() > (unsigned long)biggest[i])
				biggest[i] = flesh[i][j].length();
		}
	}
	
	for (unsigned long j = 0; j < flesh[0].size(); j++) {
		for (int z = 0; z < cols; z++) {
			printf("%s", flesh[z][j].c_str());
			if (z+1 == cols)
				continue;
			for (int y = biggest[z]-flesh[z][j].size(); y >= 0; y--) {
				putchar(' ');
			}
			putchar(sep);
		}
		putchar('\n');
	}
}

