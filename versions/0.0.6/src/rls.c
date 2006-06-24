/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <regex.h>
#include <string.h>


int hm;
char ** pkgs; 

int main(int argc, char ** argv)
{
	char in [1024];
	char * regexp = "";
	regex_t compiled;
	int result = 0;

	if (argc > 1) {
		regexp = argv[1];
	}

	if (regcomp(&compiled, regexp, REG_ICASE)) {
		fprintf(stderr, "Someting went wrong while processing the regular expresion!\n");
		return 1;
	}

	scanf("%s\n", in);
	while (strcmp(in, "-") != 0) {
		if (! (result = regexec(&compiled, in, 0, NULL, 0))) {
			printf("%s\n", in);
		}
		scanf("%s\n", in);
	}

	return 0;
}

