/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif

char * first_word(const char * cmd);

/* Doesn't return a new instance of a string! Just increases the pointer. */
char * trimleft(char * cmd);

char * trim(char * src);

long update_date(char * fn);

char * word_at_point(char *text, int point);

#ifdef __cplusplus
class word_iterator
{
public:
	word_iterator(char *_text);
	char *next_word();
	char *operator++();
private:
	const char *text;
	int len;
	char *cur;
};

class backward_word_iterator
{
public:
	backward_word_iterator(char *_text);
	char *prev_word();
	char *operator--();
private:
	const char *text;
	char *cur;
};

#endif

#ifdef __cplusplus
}
#endif

#endif


