/*
  Copyright (C) 2005 Marcin R. Wrochniak <wrochniak@gmail.com>

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
#include <fnmatch.h>
#include <string.h>

int hm;
char ** pkgs; 

int main(int argc, char ** argv)
{
	char in [1024];
	char * wildcard = argv[1];

	/* it's commented because it should behave as a normal ls command
	 * and list all items when no arguments.
	 */
	/*if (argc == 1) {
		fprintf(stderr, "Not enough parameters!\n");
		while (strcmp(in, "-") != 0) { scanf("%s\n", in); }
		return 1;
	}*/
	
	scanf("%s\n", in);
	if (argc == 1) {
		while (strcmp(in, "-") != 0) {
			puts(in);
			scanf("%s\n", in);
		}
	} else
		while (strcmp(in, "-") != 0) {
			if (! fnmatch(wildcard, in, 0)) {
				puts(in);
			}
			scanf("%s\n", in);
		}

	return 0;
}

