#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <string.h>

int hm;
char ** pkgs; 

int main(int argc, char ** argv)
{
	char in [1024];
	char * wildcard = argv[1];

	if (argc == 1) {
		fprintf(stderr, "Not enough parameters!\n");
		while (strcmp(in, "-") != 0) { scanf("%s\n", in); }
		return 1;
	}
	
	scanf("%s\n", in);
	while (strcmp(in, "-") != 0) {
		if (! fnmatch(wildcard, in, 0)) {
			printf("%s\n", in);
		}
		scanf("%s\n", in);
	}

	return 0;
}

/* vim: ts=4
*/
