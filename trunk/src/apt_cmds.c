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

#include <unistd.h>
#include <sys/stat.h>
#include <regex.h>
#include <fnmatch.h>
#include <string.h>

#include "apt_cmds.h"
#include "config_parse.h"
#include "readindex.h"
#include "string.h"

#define newcmd(WW) \
char * tmp = (char*)malloc(strlen(aptcmd)+strlen(WW)+2);\
sprintf(tmp, "%s %s", WW, aptcmd);\
system(tmp);\
free(tmp);

#define R_MSG "Reading package database...\n"

long tmpdate;

void check_a()
{
	if (!CFG_REFRESH_INDEXES && !CFG_REFRESH_INDEXES_ALL)
		return;
	
	if (update_date(CFG_UPDATE_FILE) > tmpdate) {
		free_indexes();
		printf(R_MSG);
		read_indexes();
	}
}

/* aptsh */

int apt_dump_cfg()
{
	cfg_dump();
}

int apt_help()
{
	system("man aptsh");
}

int apt_regex()
{
/*	char * regexp = trimleft(aptcmd);
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
	}*/
	FILE * pipe;
	int i;
	char * tmp = (char*)malloc(strlen(aptcmd)+strlen(SHARED_FOLDER)+5);
	sprintf(tmp, "%s%s\x0", SHARED_FOLDER, aptcmd);
	pipe = popen(tmp, "w");
	for (i =0; i < hm; i++) {
		fprintf(pipe, "%s\n", pkgs[i]);
	}
	fprintf(pipe, "-\n");
	pclose(pipe);
}

int apt_ls()
{
/*	char * wildcard = trimleft(aptcmd);
	regex_t compiled;
	int result = 0;
	int i;
	while (((*wildcard) != ' ') && ((*wildcard) != '\0')) wildcard++;
	wildcard = trimleft(wildcard);
	for (i = 0; i < hm; i++) {
		if (! fnmatch(wildcard, pkgs[i], 0)) {
			printf("%s\n", pkgs[i]);
		}
	}*/
	FILE * pipe;
	int i;
	char * tmp = (char*)malloc(strlen(aptcmd)+strlen(SHARED_FOLDER)+4);
	sprintf(tmp, "\%s%s\x0", SHARED_FOLDER, aptcmd);
	pipe = popen(tmp, "w");
	for (i =0; i < hm; i++) {
		fprintf(pipe, "%s\n", pkgs[i]);
	}
	fprintf(pipe, "-\n");
	pclose(pipe);
}

int apt_dpkg()
{
	newcmd("");
}

int apt_whichpkg()
{	
	char * cmdtmp = aptcmd;
	aptcmd = trimleft(aptcmd)+strlen("whichpkg");
	newcmd("dpkg -S");
	aptcmd = cmdtmp;
}

/* apt-get */

int apt_install()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_update()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	if (! CFG_REFRESH_INDEXES_ALL)
		return 0;
	if (update_date(CFG_UPDATE_FILE) > tmpdate)
	{
		free_indexes();
		printf(R_MSG);
		read_indexes();
	}
}

int apt_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_dselect_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_dist_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);	
	newcmd("apt-get");
	check_a();
}

int apt_remove()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_source()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_build_dep()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_check()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
}

int apt_clean()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
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

int apt_whatis()
{
	#define WHATIS_CMD "apt-cache show %s | grep ^Description | head -n 1 | sed -s 's/Description://'"
	char * tmp = (char*)malloc(strlen(aptcmd)+strlen(WHATIS_CMD));
	sprintf(tmp, WHATIS_CMD, (char*)(trimleft(aptcmd)+strlen("whatis ")));
	system(tmp);
	free(tmp);
}

/* vim: ts=4
*/

