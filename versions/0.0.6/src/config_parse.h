/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

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

struct config_option {
        char * name; /* Option's name. */
        void * value;
        char is_int; /* Is this option an integer? If so, use value as a area 
	              * for an integer, else, use as char*.
	              */
};

extern struct config_option options[];

#define OPTIONS_COUNT 9

#define CFG_PS1                     (char*)options[0].value
#define CFG_USE_HISTORY             (unsigned long int)options[1].value
#define CFG_UPDATE_FILE             (char*)options[2].value
#define CFG_REFRESH_INDEXES         (unsigned long int)options[3].value
#define CFG_REFRESH_INDEXES_ALL     (unsigned long int)options[4].value
#define CFG_HISTORY_FILE            (char*)options[5].value
#define CFG_HISTORY_COUNT           (unsigned long int)options[6].value
#define CFG_PS1_CQ_MODE             (char*)options[7].value
#define CFG_QUEUE_SIMULATE          (char*)options[8].value

extern char * config_file;

void cfg_defaults();
void cfg_dump();
int cfg_parse();

#ifdef __cplusplus
}
#endif

#endif



