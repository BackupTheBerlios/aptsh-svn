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

#include <unistd.h>
#include <sys/stat.h>
#include <regex.h>
#include <fnmatch.h>


#include "apt_cmds.h"
#include "config_parse.h"
#include "genindex.h"
#include "readindex.h"
#include "string.h"

#define newcmd(WW) \
char * tmp = (char*)malloc(strlen(aptcmd)+strlen(WW)+2);\
sprintf(tmp, "%s %s", WW, aptcmd);\
system(tmp);\
free(tmp);

void check_a()
{
	if (!CFG_REFRESH_INDEXES && !CFG_REFRESH_INDEXES_ALL)
		return;
       /* if ((access(CFG_PKG_LIST, F_OK) == -1) || (access(CFG_PKG_COUNT, F_OK) == -1) ||
            (access(CFG_PKG_LIST_INSTALLED, F_OK) == -1) || (access(CFG_PKG_COUNT_INSTALLED, F_OK) == -1)) {
                generate_index(CFG_PKG_LIST, CFG_PKG_COUNT, CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
        	free(pkgs);
        	free(pkgs_i);
        	printf("Reading package database...\n");
        	read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
        	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);		
        } else
        if (update_date(CFG_UPDATE_FILE) > update_date(CFG_PKG_LIST)) {
                generate_index(CFG_PKG_LIST, CFG_PKG_COUNT, CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
        	free(pkgs);
        	free(pkgs_i);
        	printf("Reading package database...\n");
        	read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
        	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
        }*/

	if ((access(CFG_PKG_LIST, F_OK) == -1) || (access(CFG_PKG_COUNT, F_OK) == -1)) {
		generate_index1();
		free(pkgs);
        	printf("Reading package database [1]...\n");
		read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
	} else
	if (update_date(CFG_UPDATE_FILE) > update_date(CFG_PKG_LIST)) {
		generate_index1();
		free(pkgs);
        	printf("Reading package database [1]...\n");
		read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
	}

	if ((access(CFG_PKG_LIST_INSTALLED, F_OK) == -1) || (access(CFG_PKG_COUNT_INSTALLED, F_OK) == -1)) {
		generate_index2();
        	free(pkgs_i);
        	printf("Reading package database [2]...\n");
        	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
	} else
	if (update_date(CFG_UPDATE_FILE_INSTALLED) > update_date(CFG_PKG_LIST_INSTALLED)) {
		generate_index2();
        	free(pkgs_i);
        	printf("Reading package database [2]...\n");
        	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
	}
}

/*#define check_a();\
	if (CFG_REFRESH_INDEXES || CFG_REFRESH_INDEXES_ALL) {\
        	generate_index(CFG_PKG_LIST, CFG_PKG_COUNT, CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);\
        	free(pkgs);\
        	free(pkgs_i);\
        	printf("Reading package database...\n");\
        	read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);\
        	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);\
	}
*/		

/* aptsh */

int apt_dump_cfg()
{
	cfg_dump();
}

int apt_generate_index()
{
	generate_index1();
	generate_index2();
}

int apt_help()
{
	system("man aptsh");
}

int apt_regex()
{
	char * regexp = trimleft(aptcmd);
	regex_t compiled;
	int result = 0;
	int i;
	while (((*regexp) != ' ') && ((*regexp) != '\0')) regexp++;
	regexp = trimleft(regexp);
	if (regcomp(&compiled, regexp, REG_ICASE)) {
		fprintf(stderr, "Someting went wrong while processing the regular expresion!\n");
		return 1;
	}

	for (i = 0; i < hm; i++) {
		if (! (result = regexec(&compiled, pkgs[i], 0, NULL, 0))) {
			printf("%s\n", pkgs[i]);
		}
	}
}

int apt_ls()
{
        char * wildcard = trimleft(aptcmd);
        regex_t compiled;
        int result = 0;
        int i;
        while (((*wildcard) != ' ') && ((*wildcard) != '\0')) wildcard++;
        wildcard = trimleft(wildcard);
	for (i = 0; i < hm; i++) {
		if (! fnmatch(wildcard, pkgs[i], 0)) {
			printf("%s\n", pkgs[i]);
		}
	}
}

int apt_dpkg()
{
	newcmd("");
}

/* apt-get */

int apt_install()
{
	newcmd("apt-get");
	check_a();
}

int apt_update()
{
	newcmd("apt-get");
	if (! CFG_REFRESH_INDEXES_ALL)
		return 0;
	generate_index1();
	free(pkgs);
	printf("Reading package database [1]...\n");
	read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
}

int apt_upgrade()
{
	newcmd("apt-get");
	check_a();
}

int apt_dselect_upgrade()
{
	newcmd("apt-get");
	check_a();
}

int apt_dist_upgrade()
{
	newcmd("apt-get");
	check_a();
}

int apt_remove()
{
	newcmd("apt-get");
	check_a();
}

int apt_source()
{
	newcmd("apt-get");
	check_a();
}

int apt_build_dep()
{
	newcmd("apt-get");
	check_a();
}

int apt_check()
{
	newcmd("apt-get");
}

int apt_clean()
{
	newcmd("apt-get");
	check_a();
}

int apt_autoclean()
{
	newcmd("apt-get");
}

/* apt-cache */

int apt_show()
{
	newcmd("apt-cache");
}

int apt_dump()
{
	newcmd("apt-cache");
}

int apt_add()
{
	newcmd("apt-cache");
}

int apt_showpkg()
{
	newcmd("apt-cache");
}

int apt_stats()
{
	newcmd("apt-cache");
}

int apt_showsrc()
{
	newcmd("apt-cache");
}

int apt_dumpavail()
{
	newcmd("apt-cache");
}

int apt_unmet()
{
	newcmd("apt-cache");
}

int apt_search()
{
	newcmd("apt-cache");
}

int apt_depends()
{
	newcmd("apt-cache");
}

int apt_redepends()
{
	newcmd("apt-cache");
}

int apt_pkgnames()
{
	newcmd("apt-cache");
}

int apt_dotty()
{
	newcmd("apt-cache");
}

int apt_policy()
{
	newcmd("apt-cache");
}

int apt_madison()
{
	newcmd("apt-cache");
}

