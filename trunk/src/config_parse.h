/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define OPTIONS_COUNT 10

#define CFG_PS1 (char*)options[0].value
#define CFG_USE_HISTORY (int)options[1].value
#define CFG_PKG_LIST (char*)options[2].value
#define CFG_PKG_COUNT (char*)options[3].value
#define CFG_UPDATE_FILE (char*)options[4].value
#define CFG_PKG_LIST_INSTALLED (char*)options[5].value
#define CFG_PKG_COUNT_INSTALLED (char*)options[6].value
#define CFG_REFRESH_INDEXES (char*)options[7].value
#define CFG_REFRESH_INDEXES_ALL (char*)options[8].value
#define CFG_UPDATE_FILE_INSTALLED (char*)options[9].value

char * config_file;

struct config_option {
	char * name; /* name of option */
	void * value;
	char is_int; /* is this option an integer? if so, use value as a area for integer. else, use as char* */ 
};

struct config_option options[OPTIONS_COUNT];

void cfg_defaults();
void cfg_dump();
int cfg_parse();

