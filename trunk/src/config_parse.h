/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef CONFIGPARSE_H
#define CONFIGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif
	
#define OPTIONS_COUNT 5

#define CFG_PS1 (char*)get_cfg_opt(0)
#define CFG_USE_HISTORY (int)get_cfg_opt(1)
#define CFG_UPDATE_FILE (char*)get_cfg_opt(2)
#define CFG_REFRESH_INDEXES (int)get_cfg_opt(3)
#define CFG_REFRESH_INDEXES_ALL (int)get_cfg_opt(4)


char * config_file;

void * get_cfg_opt(int index);

void cfg_defaults();
void cfg_dump();
int cfg_parse();

#ifdef __cplusplus
}
#endif

#endif


/* vim: ts=4
*/

