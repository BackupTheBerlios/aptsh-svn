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

#include <apt-pkg/error.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/init.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/cmndline.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/srcrecords.h>
#include <apt-pkg/version.h>
#include <apt-pkg/policy.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/sptr.h>

#include "apt_cmds.h"
#include "config_parse.h"
#include "read_index.h"
#include "string_utils.h"

#include "column.h"

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
		i_setsig();

		FILE * fp = popen(sth, "w");
		int len = strlen(yes);
		char * tmp = (char*)malloc(len+2);
		strcpy(tmp, yes);
		tmp[len] = '\n';
		tmp[len+1] = '\0';
		while ((fputs(tmp, fp)!=EOF));
		pclose(fp);
	}else {
		system(sth);
	}
}

long tmpdate;

// Checks whether cache file has been changed since last reading (stored in tmpdate)
void check_a()
{
	if (!CFG_REFRESH_INDEXES && !CFG_REFRESH_INDEXES_ALL)
		return;
	
	free_indexes();

	if (_config->FindB("APT::Cache::Generate", true)) {
		puts("Generating and mapping caches...");
		gen_indexes();
	} else {
		puts("Mapping caches...");
		read_indexes();
	}
}

struct command cmds[] = {
	/* apt-get */
	{ "install", apt_install, AVAILABLE, true, NULL, false, true },
	{ "update", apt_update, AVAILABLE, false, NULL, false, true },
	{ "upgrade", apt_upgrade, AVAILABLE, true, NULL, false, true },
	{ "dselect-upgrade", apt_dselect_upgrade, AVAILABLE, false, NULL, false, true },
	{ "dist-upgrade", apt_dist_upgrade, AVAILABLE, false, NULL, false, true },
	{ "remove", apt_remove, INSTALLED, true, NULL, false, true },
	{ "source", apt_source, AVAILABLE, true, NULL, false, true },
	{ "build-dep", apt_build_dep, AVAILABLE, true, NULL, false, true },
	{ "check", apt_check, AVAILABLE, false, NULL, false, true },
	{ "clean", apt_clean, AVAILABLE, false, NULL, false, true },
	{ "autoclean", apt_autoclean, AVAILABLE, false, NULL, false, true },
	/* apt-cache */
	{ "show", apt_show, AVAILABLE, true, NULL, false, false },
	{ "dump", apt_dump, AVAILABLE, false, NULL, false, false },
	{ "add", apt_add, FS, false, NULL, false, false },
	{ "showpkg", apt_showpkg, AVAILABLE, true, NULL, false, false },
	{ "stats", apt_stats, NONE, false, NULL, false, false },
	{ "showsrc", apt_showsrc, AVAILABLE, false, NULL, false, false },
	{ "dumpavail", apt_dumpavail, NONE, false, NULL, false, false },
	{ "unmet", apt_unmet, AVAILABLE, false, NULL, false, false },
	{ "search", apt_search, AVAILABLE, false, NULL, false, false },
	{ "depends", apt_depends, AVAILABLE, true, NULL, false, false },
	{ "rdepends", apt_rdepends, AVAILABLE, true, NULL, false, false },
	{ "pkgnames", apt_pkgnames, NONE, false, NULL, false, false },
	{ "dotty", apt_dotty, AVAILABLE, false, NULL, false, false },
	{ "policy", apt_policy, AVAILABLE, false, NULL, false, false },
	{ "madison", apt_madison, AVAILABLE, false, NULL, false, false },
	{ "whatis", apt_whatis, AVAILABLE, true, NULL, false, false },
	/* aptsh */
	{ "dpkg", apt_dpkg, DPKG, false, NULL, true, false },
	{ "dpkg-reconfigure", apt_dpkg_reconfigure, INSTALLED, true, "dpkg", false, false },
	{ "whichpkg", apt_whichpkg, FS, false, NULL, false, false },
	{ "listfiles", apt_listfiles, INSTALLED, true, NULL, false, false },
	{ "dump-cfg", apt_dump_cfg, FS, false, NULL, false, false },
	{ "rls", apt_regex, AVAILABLE, false, NULL, false, false },
	{ "ls", apt_ls, AVAILABLE, false, NULL, false, false },
	{ "orphans", apt_orphans, NONE, false, NULL, true, false },
	{ "orphans-all", apt_orphans_all, NONE, false, "orphans", false, false },
	{ "queue", apt_queue, NONE, false, NULL, true, false },
	{ "queue-commit", apt_queue_commit, NONE, false, "queue", false, false },
	{ "queue-commit-say", apt_queue_commit_say, NONE, false, "queue", false, false },
	{ "queue-clear", apt_queue_clear, NONE, false, "queue", false, false },
	{ "queue-remove", apt_queue_remove, NONE, false, "queue", false, false },
	{ "help", apt_help, NONE, false, NULL, false, false },
	{ "quit", NULL, NONE, false, NULL, false, false } 
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
	enum completion sort = NONE;
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

// This macro executes aptcmd command (ie. install aptsh), preceding it with shell command - WW (ie. apt-get)
char * tmp;
#define newcmd(WW) \
tmp = (char*)malloc(strlen(aptcmd)+strlen(WW)+2);\
sprintf(tmp, "%s %s", WW, aptcmd);\
realizecmd(tmp);

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
		
		if (CFG_QUEUE_SIMULATE) {
			char * fword = first_word(line_t);
			use_realcmd = 0;
			for (int i = 0; i < CMD_NUM; i++) {
				if (!strcmp(fword, cmds[i].name) && cmds[i].apt_get) {
					const char * simulator = "apt-get --simulate";
					tmp = (char*)malloc(strlen(line)+strlen(simulator)+2);
					sprintf(tmp, "%s %s", simulator, line);
					system(tmp);
					free(tmp);
					break;
				}
			}
			free(fword);
		}
		
		commit_count++;
		commitz = (char**)realloc(commitz, commit_count*sizeof(char*));

		validate(line_t);
		
		commitz[commit_count-1] = strdup(line_t);
		return 0;
	}

	if (line[0] == '.') {
		realizecmd((char*)(line+1));
		
		return 0;
	}

	char * execmd = first_word(trimleft(line));
	
	if ((! strcmp(execmd, "quit")) || (!strcmp(execmd, "exit")) || (!strcmp(execmd, "bye")))
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


/* aptsh */

int apt_dump_cfg()
{
	cfg_dump();

	return 0;
}

int apt_help()
{
	system("man aptsh");
//	realizecmd("man aptsh");

	return 0;
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

// This is used by apt_regex() and apt_ls().
#define LIBEXEC_PREFIX "aptsh_"

int apt_regex()
{
	FILE * pipe;
	char * tmp;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	i_setsig();

	// prevents from situation when user prefixes cmd with whitespaces
	char * aptcmd_t = trimleft(aptcmd);
	tmp = (char*)malloc(strlen(aptcmd_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+5);
	sprintf(tmp, "%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, aptcmd_t, '\0');
	pipe = popen(tmp, "w");
	
	while (e.end() == false) {
		fprintf(pipe, "%s\n", e.Name());
		e++;
	}	

	fprintf(pipe, "-\n");
	pclose(pipe);

	return 0;
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
	tmp = (char*)malloc(strlen(aptcmd_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+4);
	sprintf(tmp, "\%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, aptcmd_t, '\0');
	pipe = popen(tmp, "w");

	while (e.end() == false) {
		fprintf(pipe, "%s\n", e.Name());
		e++;
	}

	fprintf(pipe, "-\n");
	pclose(pipe);

	return 0;
}

int apt_dpkg()
{
	newcmd("");
	return 0;
}

int apt_dpkg_reconfigure()
{
	newcmd("");
	return 0;
}

int apt_whichpkg()
{
	char * cmdtmp = aptcmd;
	aptcmd = trimleft(aptcmd)+strlen("whichpkg");
	newcmd("dpkg -S");
	aptcmd = cmdtmp;
	
	return 0;
}

int apt_listfiles()
{
	char * cmdtmp = aptcmd;
	aptcmd = trimleft(aptcmd)+strlen("listfiles");
	newcmd("dpkg -L");
	aptcmd = cmdtmp;

	return 0;
}

// Display orphaned libraries in the system
int apt_orphans()
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	while (e.end() == false) {
		if (e->CurrentVer != 0) {
			const char * section = e.Section();
			if (section == NULL) // it can't be library, since it doesn't belong to any section. :)
				continue;
			if (strstr(section, "libs") || strstr(section, "libdevel")) {
				//printf("%s\n", e.Name());
				//pkgCache::DepIterator D = e.RevDependsList();
				//if (D.end()) {
				//	printf("%s\n", e.Name());
				//	continue;
				//}
				bool found = false;
				for (pkgCache::DepIterator D = e.RevDependsList(); D.end() == false; D++ ) {
					pkgCache::PkgIterator tmp = D.ParentPkg();
					if (tmp->CurrentVer != 0) {
						found = true;
						break;
					}
				}
				if (! found)
					printf("%s\n", e.Name());
			}
		}
		/*if (! strncmp(e.Name(), text, len)) {
			char * tmp = (char*)malloc(strlen(e.Name())+1);
			strcpy(tmp, e.Name());
			e++;
			return tmp;
		}*/
		e++;
	}

	return 0;
}

// Display all orphaned (without any reverse dependencies installed) packages in the system
int apt_orphans_all()
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	column_display * view = new column_display(2, ' ');

	while (e.end() == false) {
		if (e->CurrentVer != 0) {
			bool found = false;
			for (pkgCache::DepIterator D = e.RevDependsList(); D.end() == false; D++ ) {
				pkgCache::PkgIterator tmp = D.ParentPkg();
				if ((tmp->CurrentVer != 0) && D.IsCritical()) {
					found = true;
					break;
				}
			}
			if (! found) {
				view->add((char*)e.Section(), 0);
				view->add((char*)e.Name(), 1);
			}
		}
		e++;
	}

	view->dump();

	delete view;

	return 0;
}

int apt_queue_commit()
{
	use_realcmd = 0;

	for (int i = 0; i < commit_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, commit_count);
		execute(commitz[i], 0);
	}

	return 0;
}

int apt_queue_commit_say()
{
	
	yes = trimleft(trimleft(aptcmd)+strlen("queue-commit-say"));
	
	use_realcmd = 1;

	i_setsig();

	for (int i = 0; i < commit_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, commit_count);
		execute(commitz[i], 0);
	}

	use_realcmd = 0;

	return 0;
}

int apt_queue_clear()
{
	for (int i = 0; i < commit_count; i++) {
		free(commitz[i]);
	}
	free(commitz);
	commitz = NULL;
	commit_count = 0;
	return 0;
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

int compare_ints(const void *a, const void *b)
{
	const int * _a = (const int *)a;
	const int * _b = (const int *)b;

	return (*_a > *_b)-(*_a < *_b);
}



// Function below is used internally by apt_commit_remove() to add items to 
// internal array of items to remove from command-queue
#if __GNUC__ >= 3
inline
#endif
void add_item(const int value, int * (&arr), int & arrlen)
{
	if ((value > 0) && (value <= commit_count)) {
		/* Check whether item already exist */
		bool exists = false;
		for (int j = 0; j < arrlen; j++) {
			if (arr[j] == value) {
				/*It produces too much unnecessary mess, thus it's commented\
				  fprintf(stderr, "Hey, you can't remove item %d twice!\n", value); */
				exists = true;
			}
		}
		if (! exists) {
			arr = (int*)realloc(arr, ++arrlen*sizeof(int));
			arr[arrlen-1] = value;
		}
	} else {
		fprintf(stderr, "Item number %d doesn't exist!\n", value);
	}
}

int apt_queue_remove()
{
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
		char * end = NULL;
		if ((end = strchr(tmp, '-')) != NULL) {
			int len = end-tmp;
			char * begin = (char*)malloc(len+1);
			strncpy(begin, tmp, len);
			begin[len] = '\0';
			end++; // It contants '-', so we omit it...
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
				
			if (_begin <= _end)
				for (int i = _begin; i <= _end; i++) {
					add_item(i, arr, arrlen);
				}
			else
				for (int i = _end; i <= _begin; i++) {
					add_item(i, arr, arrlen);
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
			add_item(_tmp, arr, arrlen);
		}
	}
	free(tmp);
	qsort(arr, arrlen, sizeof(int), compare_ints);
	for (int i = arrlen-1; i >= 0; i--) {
		real_remove(arr[i]-1);
	}

	return 0;
}

int apt_queue()
{
	for (int i = 0; i < commit_count; i++) {
		printf("%d: %s\n", i+1, commitz[i]);
	}
	return 0;
}

/* apt-get */

int apt_install()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_update()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	if (! CFG_REFRESH_INDEXES_ALL)
		return 0;

	if (_config->FindB("APT::Cache::Generate", true)) {
		puts("Generating and mapping caches...");
		gen_indexes();
	} else {
		puts("Mapping caches...");
		read_indexes();
	}
	return 0;
}

int apt_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_dselect_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_dist_upgrade()
{
	tmpdate = update_date(CFG_UPDATE_FILE);	
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_remove()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_source()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_build_dep()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_check()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_clean()
{
	tmpdate = update_date(CFG_UPDATE_FILE);
	newcmd("apt-get");
	check_a();
	return 0;
}

int apt_autoclean()
{
	newcmd("apt-get");
	return 0;
}

/* apt-cache */

int apt_show()
{
	newcmd("apt-cache");
	return 0;
}

int apt_dump()
{
	newcmd("apt-cache");
	return 0;
}

int apt_add()
{
	newcmd("apt-cache");
	return 0;
}

int apt_showpkg()
{
	newcmd("apt-cache");
	return 0;
}

int apt_stats()
{
	newcmd("apt-cache");
	return 0;
}

int apt_showsrc()
{
	newcmd("apt-cache");
	return 0;
}

int apt_dumpavail()
{
	newcmd("apt-cache");
	return 0;
}

int apt_unmet()
{
	newcmd("apt-cache");
	return 0;
}

int apt_search()
{
	newcmd("apt-cache");
	return 0;
}

int apt_depends()
{
	newcmd("apt-cache");
	return 0;
}

int apt_rdepends()
{
	newcmd("apt-cache");
	return 0;
}

int apt_pkgnames()
{
	newcmd("apt-cache");
	return 0;
}

int apt_dotty()
{
	newcmd("apt-cache");
	return 0;
}

int apt_policy()
{
	newcmd("apt-cache");
	return 0;
}

int apt_madison()
{
	newcmd("apt-cache");
	return 0;
}

int apt_whatis()
{
#define WHATIS_CMD "apt-cache show %s | grep ^Description | head -n 1 | sed 's/Description://'"
	char * tmp = (char*)malloc(strlen(aptcmd)+strlen(WHATIS_CMD));
	sprintf(tmp, WHATIS_CMD, (char*)(trimleft(aptcmd)+strlen("whatis ")));
	//system(tmp);
	realizecmd(tmp);
	free(tmp);
	return 0;
}


