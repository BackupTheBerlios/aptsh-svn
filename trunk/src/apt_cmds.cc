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

#include <readline/readline.h>
#include <readline/history.h>

#include <apt-pkg/pkgcachegen.h>

#include "apt_cmds.h"
#include "config_parse.h"
#include "readindex.h"
#include "string.h"

char storing;

char * aptcmd;

char use_realcmd;

char * yes;

// Number of steps in commitlog
int commit_count;
char ** commitz;

extern void i_setsig();

void realizecmd(char * sth) {
	if (use_realcmd) {
		//char * tmp = (char*)malloc(strlen(STH)+1);
		//strcpy
		i_setsig();

		FILE * fp = popen(sth, "w");
		int len = strlen(yes);
		char * tmp = (char*)malloc(len+2);
		strcpy(tmp, yes);
		tmp[len] = '\n';
		tmp[len+1] = '\0';
		while ((fputs(tmp, fp)!=EOF)) {
		}
		pclose(fp);
	}else {
		system(sth);
	}
}

long tmpdate;

#define R_MSG "Reading package database...\n"

// Checks whether cache file has been changed since last reading (stored in tmpdate)
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

struct command
{
	char * name;
	int (*funct)();
	enum completion cpl;
	// decides whether words after command sould be validated,
	// example: "install apt dpkg kde qwertyqqw" is going to warn on "qwertyqqw",
	// because such a package doesn't exist (do_validation for "install" is YES)
	// and it checks for existence in all packages, because cpl of "install" is AVAILABLE
	char do_validation;
} cmds[] = {
#define YES 1
#define NO 0
	/* apt-get */
	{ "install", apt_install, AVAILABLE, YES },
	{ "update", apt_update, AVAILABLE, NO },
	{ "upgrade", apt_upgrade, AVAILABLE, YES },
	{ "dselect-upgrade", apt_dselect_upgrade, AVAILABLE, NO },
	{ "dist-upgrade", apt_dist_upgrade, AVAILABLE, NO },
	{ "remove", apt_remove, INSTALLED, YES },
	{ "source", apt_source, AVAILABLE, YES },
	{ "build-dep", apt_build_dep, AVAILABLE, YES },
	{ "check", apt_check, AVAILABLE, NO }, // FIXME: not sure
	{ "clean", apt_clean, AVAILABLE, NO },
	{ "autoclean", apt_autoclean, AVAILABLE, NO },
	/* apt-cache */
	{ "show", apt_show, AVAILABLE, YES },
	{ "dump", apt_dump, AVAILABLE, NO },
	{ "add", apt_add, FS, NO },
	{ "showpkg", apt_showpkg, AVAILABLE, YES },
	{ "stats", apt_stats, NONE, NO },
	{ "showsrc", apt_showsrc, AVAILABLE, NO },
	{ "dumpavail", apt_dumpavail, NONE, NO },
	{ "unmet", apt_unmet, AVAILABLE, NO },
	{ "search", apt_search, AVAILABLE, NO },
	{ "depends", apt_depends, AVAILABLE, YES },
	{ "rdepends", apt_rdepends, AVAILABLE, YES },
	{ "pkgnames", apt_pkgnames, NONE, NO },
	{ "dotty", apt_dotty, AVAILABLE, NO },
	{ "policy", apt_policy, AVAILABLE, NO },
	{ "madison", apt_madison, AVAILABLE, NO },
	{ "whatis", apt_whatis, AVAILABLE, YES },
	/* aptsh */
	{ "dpkg", apt_dpkg, FS, NO },
	{ "whichpkg", apt_whichpkg, FS, NO },
	{ "listfiles", apt_listfiles, INSTALLED, YES },
	{ "dump-cfg", apt_dump_cfg, FS, NO },
	{ "rls", apt_regex, AVAILABLE, NO },
	{ "ls", apt_ls, AVAILABLE, NO },
	{ "commit", apt_commit, NONE, NO },
	{ "commit-say", apt_commit_say, NONE, NO },
	{ "commit-clear", apt_commit_clear, NONE, NO },
	{ "commit-remove", apt_commit_remove, NONE, NO },
	{ "commit-status", apt_commit_status, NONE, NO },
	{ "help", apt_help, NONE, NO },
	{ "quit", NULL, NONE, NO } 
};

// Check whether package exists
// ex decides which packages ought to be checked:
//   AVAILABLE - all packages (default)
//   INSTALLED  - only installed packages
bool package_exists(char * name, enum completion ex = AVAILABLE)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;

	Cache = new pkgCache(m);

	for (e = Cache->PkgBegin(); e.end() == false; e++) {
		if (ex == INSTALLED) {
			pkgCache::Package * ppk = (pkgCache::Package *)e;
			// 6 means installed
			if (ppk->CurrentState == 6) {
				if (! strcmp(e.Name(), name)) {
					return true;
				}
			}else {
				continue;
			}
		}
		if (! strcmp(e.Name(), name)) {
			//e++;
			return true;
		}
	}
	return false;
}

// Validates aptsh commadn
// You can't give a command with ';' at the begining, if so it will throw up a warning
// Also, command must be already trimmed
int validate(char * cmd)
{
	//cmd = trimleft(cmd); <- it should be trimmed in execute()

	// Well, we don't try to validate shell commands
	if (cmd[0] == '.')
		return 0;
	char * tmp = first_word(cmd);
	char ok = 0;
	enum completion sort;
	char do_validation = 0;
	for (int i = 0; i < CMD_NUM; i++) {
		if (! strcmp(tmp, cmds[i].name)) {
			ok = 1;
			sort = cmds[i].cpl;
			do_validation = cmds[i].do_validation;
			break;
		}
	}
	if (! ok) {
		fprintf(stderr, "Warning: Unknown command: %s\n", tmp);
		free(tmp);
		return 1;
	}
	ok = 0;

	char *cmd2 = cmd+strlen(tmp);

	if (do_validation)
	if (sort == AVAILABLE) {
		while (1) {
			free(tmp);
			cmd2 = trimleft(cmd2);
			tmp = first_word(cmd2);
			if (! strcmp(tmp, ""))
				break;
			//printf("P: %s -\n", tmp);

			// it may be a parameter for apt, not a pkg's name
			if (tmp[0] == '-') {
				cmd2 = cmd2+strlen(tmp);
				continue;
			}

			if (! package_exists(tmp)) {
				fprintf(stderr, "Warning: Package doesn't exist: %s\n", tmp);
			}
			cmd2 = cmd2+strlen(tmp);
		}
	} else
	if (sort == INSTALLED) {
		while (1) {
			free(tmp);
			cmd2 = trimleft(cmd2);
			tmp = first_word(cmd2);
			if (! strcmp(tmp, ""))
				break;
			
			// it may be a parameter for apt, not a pkg's name
			if (tmp[0] == '-') {
				cmd2 = cmd2+strlen(tmp);
				continue;
			}

			if (! package_exists(tmp, INSTALLED)) {
				fprintf(stderr, "Warning: Package is not installed: %s\n", tmp);
			}
			cmd2 = cmd2+strlen(tmp);
		}
	}

	free(tmp);
	return 0;
}

// Returns >0 when user wants to exit
int execute(char * line, char addhistory)
{
	if (CFG_USE_HISTORY && addhistory)
		if (line && strcmp("", trimleft(line)) &&
		/* below it check whether history is empty, if so it allows to add new entry, else it checks
		   whether this line was added recently - if not, it allows to add n.e. */
		( history_list() == NULL ? 1 : strcmp(history_list()[history_length-1]->line, line) )){
			add_history(line);
			if (CFG_HISTORY_COUNT)
				if ((access(CFG_HISTORY_FILE, F_OK) == -1))
					write_history(CFG_HISTORY_FILE);
				else
					append_history(1, CFG_HISTORY_FILE);
		}
	
	if (trimleft(line)[0] == '`') {
		storing = !storing;
		return 0;
	}

	if (storing) {
		char * line_t = trimleft(trimleft(line));

		commit_count++;
		commitz = (char**)realloc(commitz, commit_count*sizeof(char*));

		validate(line_t);
		//                     line contains ' sign, so we don't need to allocate +1 bytes.
		
		commitz[commit_count-1] = strdup(line_t);
		//commitz[commit_count-1] = (char*)malloc(strlen(line_t));
		//strcpy(commitz[commit_count-1], line_t);
		return 0;
	}

	if (line[0] == '.') {
		//system((char*)(line+1));
		realizecmd((char*)(line+1));
		
		//continue;
		return 0;
	}

	char * execmd = first_word(trimleft(line));
	
	if ((! strcmp(execmd, "quit")) || (!strcmp(execmd, "exit")) || (!strcmp(execmd, "bye")))
		//break;
		return 1;
		
	
	aptcmd = line;
	char help = 0;
	for (int i = 0; i < CMD_NUM; i++) {
		if (! strcmp(execmd, cmds[i].name)) {
			if (cmds[i].funct != NULL)
				(*(cmds[i].funct))();
			help = 1;
			break;
		}
	}
	if (!help) {
		fprintf(stderr, "No such command! See 'help'.\n");
	}
	
	free(execmd);
	
	return 0;
}

// This macro executes aptcmd command (ie. install aptsh), preceding it with shell command - WW (ie. apt-get)
char * tmp;
#define newcmd(WW) \
tmp = (char*)malloc(strlen(aptcmd)+strlen(WW)+2);\
sprintf(tmp, "%s %s", WW, aptcmd);\
/*system(tmp);*/\
realizecmd(tmp);\
/*free(tmp);*/

/* aptsh */

int apt_dump_cfg()
{
	cfg_dump();
}

int apt_help()
{
	system("man aptsh");
//	realizecmd("man aptsh");
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

	// prevents from situation when user prefixes cmd with whitespaces
	char * aptcmd_t = trimleft(aptcmd);
	tmp = (char*)malloc(strlen(aptcmd_t)+strlen(SHARED_FOLDER)+5);
	sprintf(tmp, "%s%s\x0", SHARED_FOLDER, aptcmd_t);
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
	
	// prevents from situation when user prefixes cmd with whitespaces
	char * aptcmd_t = trimleft(aptcmd);
	tmp = (char*)malloc(strlen(aptcmd_t)+strlen(SHARED_FOLDER)+4);
	sprintf(tmp, "\%s%s\x0", SHARED_FOLDER, aptcmd_t);
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
	use_realcmd = 0;

	for (int i = 0; i < commit_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, commit_count);
		execute(commitz[i], 0);
	}
}

int apt_commit_say()
{
	
	yes = trimleft(trimleft(aptcmd)+strlen("commit-say"));
	//char * yes = (char*)malloc(strlen(SHARED_FOLDER)+strlen(tmp)+strlen("csay ")+1);
	//sprintf(yes, "%scsay %s", SHARED_FOLDER, tmp);
	
	use_realcmd = 1;

	i_setsig();

	for (int i = 0; i < commit_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, commit_count);
		//realcmd = NULL;
		execute(commitz[i], 0);
	}

	//free(yes);
	use_realcmd = 0;
}

int apt_commit_clear()
{
	for (int i = 0; i < commit_count; i++) {
		free(commitz[i]);
	}
	free(commitz);
	commitz = NULL;
	commit_count = 0;
	return 0;
}

// used by qsort
void swap(int * a, int * b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void qsort(int z[], int beg, int end)
{
	if (end > beg+1) {
		int p = z[beg];
		int l = beg+1;
		int r = end;
		while (l < r) {
			if (z[l] <= p) {
				l++;
			} else {
				swap(&z[l], &z[--r]);
			}
		}
		swap(&z[beg], &z[--l]);
		qsort(z, beg, l);
		qsort(z, r, end);
	}
}

// removes item from commitz
void real_remove(int num)
{
	int hm = num;
	free(commitz[hm]);
	while (hm < (commit_count-1)) {
		commitz[hm] = commitz[hm+1];
		hm++;
	}
 	commit_count--;
	commitz = (char**)realloc(commitz, commit_count*sizeof(char*));
}

int apt_commit_remove()
{
// It ought not to be used outside this function. It's just internal macro
#define add_item(VALUE)\
	if ((VALUE > 0) && (VALUE <= commit_count)) {\
		/* Check whether item already exist */\
		bool exists = false;\
		for (int j = 0; j < arrlen; j++) {\
			if (arr[j] == VALUE) {\
				/*It produces too much unnecessary mess, thus it's commented\
				  fprintf(stderr, "Hey, you can't remove item %d twice!\n", value); */\
				exists = true;\
			}\
		}\
		if (! exists) {\
			arr = (int*)realloc(arr, ++arrlen*sizeof(int));\
			arr[arrlen-1] = VALUE;\
		}\
	} else {\
		fprintf(stderr, "Item number %d doesn't exist!\n", VALUE);\
	}
	
	char * cmd = trimleft(aptcmd);
	char * tmp = first_word(cmd);
	cmd = cmd+strlen(tmp);
	int arrlen = 0;
	int * arr = NULL;
	while (1) {
		free(tmp);
		cmd = trimleft(cmd);
		tmp = first_word(cmd);
		if (! strcmp(tmp, ""))
			break;
		cmd = cmd+strlen(tmp);
		char * end;
		if (end = strstr(tmp, "-")) {
			int len = end-tmp;
			char * begin = (char*)malloc(len+1);
			strncpy(begin, tmp, len);
			begin[len] = '\0';
			end++; // It contants '-', so we omit it...
			// printf("%s - %s\n\n", begin, end);
			int _begin;
			if (! strcmp(begin, "last")) {
				_begin = commit_count;
			} else {
				_begin = atoi(begin);
			}
			
			int _end;
			if (! strcmp(end, "last")) {
				_end = commit_count;
			} else {
				_end = atoi(end);
			}
				
			if (begin <= end)
				for (int i = _begin; i <= _end; i++) {
					add_item(i);
				}
			else
				for (int i = _end; i >= _begin; i--) {
					add_item(i);
				}
			free(begin);
		} else {
			// add_item is a macro, so it would be converted MANY times without this
			int _tmp;
			if (! strcmp(tmp, "last")) {
				_tmp = commit_count;
			} else {
				_tmp = atoi(tmp);
			}
			add_item(_tmp);
		}
	}
	free(tmp);
	qsort(arr, 0, arrlen);
	for (int i = arrlen-1; i >= 0; i--) {
		//printf("%d\n", arr[i]);
		real_remove(arr[i]-1);
	}
}

int apt_commit_status()
{
	
	for (int i = 0; i < commit_count; i++) {
		printf("%d: %s\n", i+1, commitz[i]);
	}
	/*
	for (; iterator->next != NULL; iterator = iterator->next) {
		printf("%d: %s\n", ++i, iterator->text);
	}
	printf("%d: %s\n", ++i, iterator->text);
	*/
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

int apt_rdepends()
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
	//system(tmp);
	realizecmd(tmp);
	free(tmp);
}


