/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "readindex.h"


int read_index1(char * filename, char * hm_file)
{
#define BODY(X1, X2) \
	X2 = 0;\
	int count = 0;\
	int i;\
	char tmp[1023];\
	\
	FILE * fp;\
	if ((fp = fopen(hm_file, "r")) == NULL) {\
		fprintf(stderr, "Error reading indexes! (packages count)\n");\
		return 1;\
	}\
	fscanf(fp, "%d", &X2);\
	X1 = (char**)malloc(X2*sizeof(char*));\
	if ((fp = fopen(filename, "r")) == NULL) {\
		fprintf(stderr, "Error reading indexes! (package base)\n");\
		return 1;\
	}\
	while (fscanf(fp, "%s\n", tmp) != EOF) {\
		X1[count] = (char*)malloc(strlen(tmp)+1);\
		strcpy(X1[count], tmp);\
		count++;\
	}\
	fclose(fp);

	BODY(pkgs, hm)
}

int read_index2(char * filename, char * hm_file)
{
	BODY(pkgs_i, hm_i)
}
