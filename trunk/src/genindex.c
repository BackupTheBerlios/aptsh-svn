/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "genindex.h"
#include "config_parse.h"

/* generate index of all packages */
int generate_index1()
{
	char cmd[1024];
	printf("Generating indexes [1]...\n");
	sprintf(cmd, "apt-cache dump | grep ^Package | cut -d\\  -f 2 >%s && wc -l %s | cut -d\\  -f 1 >%s", CFG_PKG_LIST, CFG_PKG_LIST, CFG_PKG_COUNT);
	system(cmd);
}

/* generate index of installed packages */
int generate_index2()
{
	char cmd[1024];
	printf("Generating indexes [2]...\n");
	sprintf(cmd, "cat /var/lib/dpkg/available | grep ^Package | cut -d\\  -f 2 > %s && wc -l %s | cut -d\\  -f 1 >%s", CFG_PKG_LIST_INSTALLED, CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
	system(cmd);
}
