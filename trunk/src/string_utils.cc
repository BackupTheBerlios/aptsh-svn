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
#include <string.h>
#include <sys/stat.h>

#include "string_utils.h"

/* Returns pointer to a _new_ string contaning the first word of *cmd. */
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

/* Returns pointer to a left-trimmed cmd string. It doesn't make a new copy of it! */
char * trimleft(char * cmd)
{
	char * tmp = cmd;
	while (*tmp++ == ' ');
	return --tmp;
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

long update_date(char * fn)
{
	struct stat st;
	stat(fn, &st);
	return st.st_mtime;
}

/* Returns a word which contains the char at 'point' position. */
char * word_at_point(char *text, int point)
{
	int front = point;
	int back = point;
	char * ret;// = (char*)malloc(back-front+1);

	if (text[point] == ' ')
		return " ";

	while (text[front] != ' ' && front >= 0) {
		front--;
	}
	front++;

	while (text[back] != ' ' && text[back] != '\0') {
		back++;
	}
	back--;

	ret = (char*)malloc(back-front+2);
	strncpy(ret, text+front, back-front+1);
	ret[back-front+1] = '\0';

	return ret;
}

word_iterator::word_iterator(char *_text) : text(_text)
{
	cur = trimleft((char*)_text);
	len = strlen(_text);
}

char * word_iterator::next_word()
{
	char * fword = first_word(cur);
	cur = trimleft(cur+strlen(fword));
	return strlen(fword) ? fword : NULL;
}

char * word_iterator::operator++()
{
	return next_word();
}

backward_word_iterator::backward_word_iterator(char *_text) : text(_text)
{
	cur = &((char*)text)[strlen(text)-1];
	while ((cur >= text) && strchr(" \t", *cur)) {
		cur--;
	}
}

char *backward_word_iterator::prev_word()
{
	if (cur <= text)
		return NULL;

	char *begin = cur;
	while ((begin >= text) && !strchr(" \t", *begin)) {
		begin--;
	}
	
	char *ret = (char*)malloc(cur-begin+1);
	strncpy(ret, begin+1, cur-begin);
	ret[cur-begin] = '\0';
	
	cur = begin;
	while ((cur >= text) && strchr(" \t", *cur)) {
		cur--;
	}

	return ret;
}

char *backward_word_iterator::operator--()
{
	return prev_word();
}


