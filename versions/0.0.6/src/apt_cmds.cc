/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

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

/* Command that's actually being executed. */
char * actual_command;

bool use_realcmd;

/* Input for commands executed from realizecmd(). */
char * yes;

/* Command queue mode - just saving commands to command queue. */
bool command_queue_mode;
/* Size of command queue. */
int command_queue_count;
/* Items in command queue. */
char ** command_queue_items;


/* We should ignore SIGPIPE.
 * If we don't, aptsh is going to terminate when user creates a bad pipe. 
 * Example:
 * rls | less
 * and user terminates 'less' before it reaches end of output.
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

/* Prefix of executables in libexec directory (aptsh_{ls,rls,printer}). */
#define LIBEXEC_PREFIX "aptsh_"


/* Little helper for commands that execute external command.
 * If 'use_realcmd' is true, intput of executed command will be feeded by 
 * content of 'yes'. Otherwise, command will be just executed from system().
 */
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


/* This function executes actual_command command (ie. install aptsh),
 * preceding it with shell command - 'exec' (ie. apt-get).
 */
void newcmd(char *exec)
{
	char * tmp;
	tmp = (char*)malloc(strlen(actual_command)+strlen(exec)+2);
	sprintf(tmp, "%s %s", exec, actual_command);
	realizecmd(tmp);
	free(tmp);
}

long tmpdate;

/* Checks whether cache file has been changed since last reading (stored in tmpdate). */
/* TODO: It's possible that it's not needed any more, needs checking. */
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
	{ "help", apt_help, NONE, false, NULL, true, false },
	{ "help-howto", apt_help_howto, NONE, false, "help", false, false },
	{ "quit", NULL, NONE, false, NULL, false, false },
};

/* Check whether package exists.
 * 'ex' decides which packages ought to be checked:
 *   AVAILABLE - all packages (default)
 *   INSTALLED - only installed packages.
 */
bool package_exists(char * name, enum completion ex = AVAILABLE)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;

	Cache = new pkgCache(m);

	for (e = Cache->PkgBegin(); e.end() == false; e++) {
		if (ex == INSTALLED) {
			pkgCache::Package * ppk = (pkgCache::Package *)e;
			/* 6 means installed. */
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

/* Validates Aptsh command. */
/* Command must be already trimmed. */
int validate(char * cmd)
{
#if 0
	/* It should be already trimmed in execute(). */
	//cmd = trimleft(cmd);
#endif

	/* Well, we don't try to validate shell commands. */
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

			/* It may be a parameter for apt, not a pkg's name. */
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
			
			/* It may be a parameter for apt, not a pkg's name. */
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

/* Returns >0 when user wants to exit. */
int execute(char * line, char addhistory)
{
	if (CFG_USE_HISTORY && addhistory)
		if (line && strcmp("", trimleft(line)) &&
		/* Below it checks whether history is empty, if so it allows to add new entry,
		 * else it checks whether this line was added recently - if not, it allows to add n.e.
		 */
		( history_list() == NULL ? 1 : strcmp(history_list()[history_length-1]->line, line) )){
			add_history(line);
			if (CFG_HISTORY_COUNT)
				if ((access(CFG_HISTORY_FILE, F_OK) == -1))
					write_history(CFG_HISTORY_FILE);
				else
					append_history(1, CFG_HISTORY_FILE);
		}
	
	if (trimleft(line)[0] == '`') {
		command_queue_mode = !command_queue_mode;
		return 0;
	}

	if (command_queue_mode) {
		char * line_t = trimleft(trimleft(line));
		
		if (CFG_QUEUE_SIMULATE) {
			char * fword = first_word(line_t);
			use_realcmd = 0;
			for (int i = 0; i < CMD_NUM; i++) {
				if (!strcmp(fword, cmds[i].name) && cmds[i].apt_get) {
					const char * simulator = "apt-get --simulate";
					char *tmp = (char*)malloc(strlen(line)+strlen(simulator)+2);
					sprintf(tmp, "%s %s", simulator, line);
					system(tmp);
					free(tmp);
					break;
				}
			}
			free(fword);
		}
		
		command_queue_count++;
		command_queue_items = (char**)realloc(command_queue_items, command_queue_count*sizeof(char*));

		validate(line_t);
		
		command_queue_items[command_queue_count-1] = strdup(line_t);
		return 0;
	}

	if (line[0] == '.') {
		realizecmd((char*)(line+1));
		
		return 0;
	}

	char * execmd = first_word(trimleft(line));
	
	if ((! strcmp(execmd, "quit")) || (!strcmp(execmd, "exit")) || (!strcmp(execmd, "bye")))
		return 1;
		
	
	actual_command = line;
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


/* Aptsh's specific commands. */

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

int apt_help_howto()
{
	char * cmdtmp = actual_command;
	actual_command = trimleft(actual_command)+strlen("help-howto");
	newcmd("zcat /usr/share/doc/aptsh/HOWTO.gz");
	actual_command = cmdtmp;

//	system("zcat /usr/share/doc/aptsh/howto.gz");
//	realizecmd("man aptsh");

	return 0;
}

int apt_regex()
{
	FILE * pipe;
	char * tmp;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	i_setsig();

	/* Prevent from situation when user prefixes cmd with whitespaces. */
	char * actual_command_t = trimleft(actual_command);
	tmp = (char*)malloc(strlen(actual_command_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+5);
	sprintf(tmp, "%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, actual_command_t, '\0');
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
	
	/* Prevents from situation when user prefixes cmd with whitespaces. */
	char * actual_command_t = trimleft(actual_command);
	tmp = (char*)malloc(strlen(actual_command_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+4);
	sprintf(tmp, "\%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, actual_command_t, '\0');
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
	char * cmdtmp = actual_command;
	actual_command = trimleft(actual_command)+strlen("whichpkg");
	newcmd("dpkg -S");
	actual_command = cmdtmp;
	
	return 0;
}

int apt_listfiles()
{
	char * cmdtmp = actual_command;
	actual_command = trimleft(actual_command)+strlen("listfiles");
	newcmd("dpkg -L");
	actual_command = cmdtmp;

	return 0;
}

/* Display orphaned libraries in the system. */
int apt_orphans()
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	/* FIXME: This probably causues a memleak (we lose control
	 * on whitespaces before actual_command_t).
	 */
	char *actual_command_t = trimleft(actual_command);
	char *fw = first_word(actual_command_t);
	actual_command_t += strlen(fw);
	//actual_command_t = trimleft(actual_command_t);
	char *tmp = (char*)malloc(strlen(actual_command_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+strlen("printer ")+4);
	sprintf(tmp, "\%s%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, "printer ", actual_command_t, '\0');
	free(fw);
	FILE *pipe = popen(tmp, "w");

	while (e.end() == false) {
		if (e->CurrentVer != 0) {
			const char * section = e.Section();
			if (section == NULL) /* It can't be library, since it doesn't belong to any section. */
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
					fprintf(pipe, "%s\n", e.Name());
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

	pclose(pipe);
	free(tmp);
	
	return 0;
}

/* Display all orphaned (without any reverse dependencies installed) packages in the system. */
int apt_orphans_all()
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	/* FIXME: This probably causues a memleak (we lose control
	* on whitespaces before actual_command_t).
	*/
	char *actual_command_t = trimleft(actual_command);
	char *fw = first_word(actual_command_t);
	actual_command_t += strlen(fw);
	//actual_command_t = trimleft(actual_command_t);
	char *tmp = (char*)malloc(strlen(actual_command_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+strlen("printer ")+4);
	sprintf(tmp, "\%s%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, "printer ", actual_command_t, '\0');
	free(fw);
	FILE *pipe = popen(tmp, "w");

	column_display * view = new column_display(2, ' ', pipe);

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

	pclose(pipe);
	free(tmp);
	
	delete view;

	return 0;
}

int apt_queue_commit()
{
	use_realcmd = 0;

	for (int i = 0; i < command_queue_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, command_queue_count);
		execute(command_queue_items[i], 0);
	}

	return 0;
}

int apt_queue_commit_say()
{
	
	yes = trimleft(trimleft(actual_command)+strlen("queue-commit-say"));
	
	use_realcmd = 1;

	i_setsig();

	for (int i = 0; i < command_queue_count; i++) {
		printf(" >>> Doing step %d of %d...\n", i+1, command_queue_count);
		execute(command_queue_items[i], 0);
	}

	use_realcmd = 0;

	return 0;
}

int apt_queue_clear()
{
	for (int i = 0; i < command_queue_count; i++) {
		free(command_queue_items[i]);
	}
	free(command_queue_items);
	command_queue_items = NULL;
	command_queue_count = 0;
	return 0;
}

/* Removes item from 'command_queue_items'. */
void real_remove(int num)
{
	int hm = num;
	free(command_queue_items[hm]);
	while (hm < (command_queue_count-1)) {
		command_queue_items[hm] = command_queue_items[hm+1];
		hm++;
	}
 	command_queue_count--;
	command_queue_items = (char**)realloc(command_queue_items, command_queue_count*sizeof(char*));
}

int compare_ints(const void *a, const void *b)
{
	const int * _a = (const int *)a;
	const int * _b = (const int *)b;

	return (*_a > *_b)-(*_a < *_b);
}



/* Function below is used internally by apt_commit_remove() to add items to 
 * internal array of items to remove from command-queue.
 */
inline void add_item(const int value, int * (&arr), int & arrlen)
{
	if ((value > 0) && (value <= command_queue_count)) {
		/* Check whether item already exist */
		bool exists = false;
		for (int j = 0; j < arrlen; j++) {
			if (arr[j] == value) {
				/* It produces too much unnecessary mess, thus it's commented
				 * fprintf(stderr, "Hey, you can't remove item %d twice!\n", value);
				 */
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
	char * cmd = trimleft(actual_command);
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
			end++; /* It contants '-', so we omit it. */
			int _begin;
			if (! strcmp(begin, "last")) {
				_begin = command_queue_count;
			} else {
				_begin = atoi(begin);
			}
			
			int _end;
			if (! strcmp(end, "last")) {
				_end = command_queue_count;
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
			/* add_item is a macro, so it would be converted MANY times without this. */
			int _tmp;
			if (! strcmp(tmp, "last")) {
				_tmp = command_queue_count;
			} else {
				_tmp = atoi(tmp);
			}
			add_item(_tmp, arr, arrlen);
		}
	}
	free(tmp);
	qsort(arr, arrlen, sizeof(int), compare_ints);
	/* This is not too efficient - complexity of removing is about O(n^2).
	 * It can be done with O(n) complexity, but O(n^2) should be enough.
	 */
	for (int i = arrlen-1; i >= 0; i--) {
		real_remove(arr[i]-1);
	}

	return 0;
}

int apt_queue()
{
	for (int i = 0; i < command_queue_count; i++) {
		printf("%d: %s\n", i+1, command_queue_items[i]);
	}
	return 0;
}

/* Apt-get specific commands. */

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

/* Apt-cache specific commands. */

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
	static char *whatis_shell_command = "apt-cache show %s | grep ^Description | head -n 1 | sed 's/Description://'";
	char * tmp = (char*)malloc(strlen(actual_command)+strlen(whatis_shell_command));
	sprintf(tmp, whatis_shell_command, (char*)(trimleft(actual_command)+strlen("whatis ")));
	//system(tmp);
	realizecmd(tmp);
	free(tmp);
	return 0;
}


