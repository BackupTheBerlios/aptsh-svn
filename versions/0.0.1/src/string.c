/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>

#include "string.h"

/* returns a pointer to _new_ string contaning the first word of *cmd */
char * first_word(const char * cmd)
{
	int i = 0;
	int len = strlen(cmd);
	char * tmp = NULL;
	for (; i < len; i++) {
		if (cmd[i] == ' ') {
			tmp = (char*)malloc(i+1);
			strncpy(tmp, cmd, i);
			tmp[i] = '\0';
			return tmp;
		}
	}
	tmp = (char*)malloc(strlen(cmd)+1);
	strcpy(tmp, cmd);
	return tmp;
}

/* returns a pointer to a left-trimmed cmd string. it doesn't make a new copy of it! */
char * trimleft(char * cmd)
{
	char * tmp = cmd-1;
	while (*(++tmp) == ' ') {}
	return tmp;
}

char * trim(char * src)
{
	int len = strlen(src);
	int trimleft = 0;
	int trimright = len-1;
	char * tmp = NULL;
	while ((src[trimleft] == ' ') || (src[trimleft] == '\t')) { trimleft++; }
	if (src[trimleft] == '\0') {
		tmp = (char*)malloc(1);
		tmp[0] = '\0';
		return tmp;
	}
	while ((src[trimright] == ' ') || (src[trimright] == '\t')) { trimright--;  }
	tmp = (char*)malloc(len-trimleft-(len-trimright)+3);
	strncpy(tmp, src+trimleft, len-trimleft-(len-trimright)+1);
	return tmp;
}
