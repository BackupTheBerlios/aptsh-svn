/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

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

char * trimleft(char * cmd);

char * trim(char * src);

long update_date(char * fn);

#ifdef __cplusplus
}
#endif

#endif


