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
#include <signal.h>

#include <apt-pkg/pkgcachegen.h>

#include "apt_cmds.h"
#include "config_parse.h"
#include "readindex.h"
#include "string.h"

char * tmp;
#define newcmd(WW) \
tmp = (char*)malloc(strlen(aptcmd)+strlen(WW)+2);\
sprintf(tmp, "%s %s", WW, aptcmd);\
system(tmp);\
free(tmp);

#define R_MSG "Reading package database...\n"

char * aptcmd;

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
// Who knows whether code beneath works...
/*	pkgCache * Cache;
	pkgCache::PkgIterator e;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();

	while (e.end() == false) {
		pkgCache::Package * ppk = (pkgCache::Package *)e;
		// (1 << 1) is pkgFLAG_New
		if (ppk->Flags & (1 << 1)) {
			printf("%s\n", e.Name());
		}
		e++;
	}	
*/
// Who knows whether code beyond works...

	system("man aptsh");
}


/* We should ignore SIGPIPE.
 * If we don't, aptsh is going to terminate when user creates a bad pipe. 
 * Example:
 * rls | less
 * and user escapes from less before it reaches end of list.
 */
void i_setsig()
{
	static char set = 0;
	struct sigaction act;
	if (! set) {
		act.sa_handler = SIG_IGN;
		act.sa_flags = 0;
		sigaction(SIGPIPE, &act, NULL);
		set = 1;
	}
}

int apt_regex()
{
	FILE * pipe;
	char * tmp;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	i_setsig();

	tmp = (char*)malloc(strlen(aptcmd)+strlen(SHARED_FOLDER)+5);
	sprintf(tmp, "%s%s\x0", SHARED_FOLDER, aptcmd);
	pipe = popen(tmp, "w");
	
	//while (i < hm) {
	while (e.end() == false) {
		//i++;
		fprintf(pipe, "%s\n", e.Name());
		e++;
	}	

	fprintf(pipe, "-\n");
	pclose(pipe);
}

int apt_ls()
{
	FILE * pipe;
	char * tmp;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	i_setsig();

	tmp = (char*)malloc(strlen(aptcmd)+strlen(SHARED_FOLDER)+4);
	sprintf(tmp, "\%s%s\x0", SHARED_FOLDER, aptcmd);
	pipe = popen(tmp, "w");

	while (e.end() == false) {
		fprintf(pipe, "%s\n", e.Name());
		e++;
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

int apt_listfiles()
{
	char * cmdtmp = aptcmd;
	aptcmd = trimleft(aptcmd)+strlen("listfiles");
	newcmd("dpkg -L");
	aptcmd = cmdtmp;
}

int apt_commit()
{
	if (first == NULL)
		return 0;
	struct commit_item * iterator = first;
	struct commit_item * n;
	while (1) {
		n = iterator->next;
		free(iterator);
		if (n->next == NULL) {
			free(n);
			first = NULL;
			break;
		}
		iterator = n;
	}
}

int apt_commit_status()
{
	if (first == NULL)
		return 0;
	struct commit_item * iterator = first;
	for (; iterator->next != NULL; iterator = iterator->next) {
		printf(" %s\n", iterator->text);
	}
	printf(" %s\n", iterator->text);
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
#define WHATIS_CMD "apt-cache show %s | grep ^Description | head -n 1 | sed 's/Description://'"
	char * tmp = (char*)malloc(strlen(aptcmd)+strlen(WHATIS_CMD));
	sprintf(tmp, WHATIS_CMD, (char*)(trimleft(aptcmd)+strlen("whatis ")));
	system(tmp);
	free(tmp);
}


