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
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <apt-pkg/pkgcachegen.h>

#include "readindex.h"
#include "apt_cmds.h"
#include "string.h"
#include "config_parse.h"

#define CMD_NUM 37

// These variables are used to store commit log.
//int cl_size; // Number of logged commands
//char ** commitlog; // Logged commands

struct commit_item * commitlog;
struct commit_item * first;


struct command
{
	char * name;
	int (*funct)();
	enum completion cpl;
} cmds[] = {
	/* apt-get */
	{ "install", apt_install, AVAILABLE },
	{ "update", apt_update, AVAILABLE },
	{ "upgrade", apt_upgrade, AVAILABLE },
	{ "dselect-upgrade", apt_dselect_upgrade, AVAILABLE },
	{ "dist-upgrade", apt_dist_upgrade, AVAILABLE },
	{ "remove", apt_remove, INSTALLED },
	{ "source", apt_source, AVAILABLE },
	{ "build-dep", apt_build_dep, AVAILABLE },
	{ "check", apt_check, AVAILABLE },
	{ "clean", apt_clean, AVAILABLE },
	{ "autoclean", apt_autoclean, AVAILABLE },
	/* apt-cache */
	{ "show", apt_show, AVAILABLE },
	{ "dump", apt_dump, AVAILABLE },
	{ "add", apt_add, FS },
	{ "showpkg", apt_showpkg, AVAILABLE },
	{ "stats", apt_stats, NONE },
	{ "showsrc", apt_showsrc, AVAILABLE },
	{ "dumpavail", apt_dumpavail, NONE },
	{ "unmet", apt_unmet, AVAILABLE },
	{ "search", apt_search, AVAILABLE },
	{ "depends", apt_depends, AVAILABLE },
	{ "redepends", apt_redepends, AVAILABLE },
	{ "pkgnames", apt_pkgnames, NONE },
	{ "dotty", apt_dotty, AVAILABLE },
	{ "policy", apt_policy, AVAILABLE },
	{ "madison", apt_madison, AVAILABLE },
	{ "whatis", apt_whatis, AVAILABLE },
	/* aptsh */
	{ "dpkg", apt_dpkg, FS },
	{ "whichpkg", apt_whichpkg, FS },
	{ "listfiles", apt_listfiles, INSTALLED },
	{ "dump-cfg", apt_dump_cfg, FS },
	{ "rls", apt_regex, AVAILABLE },
	{ "ls", apt_ls, AVAILABLE },
	{ "commit", apt_commit, NONE },
	{ "commit-status", apt_commit_status, NONE },
	{ "help", apt_help, NONE },
	{ "quit", NULL, NONE } 
};

/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* packages completion */


//pkgCache * Cache;
//pkgCache::PkgIterator e;


char * cpl_pkg(const char * text, int state)
{
	static int len;
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	if (! state) {
		Cache = new pkgCache(m);
		e = Cache->PkgBegin();
		len = strlen(text);
		i = 0;
	}

	//while (i < hm) {
	while (e.end() == false) {
		//i++;
		if (! strncmp(e.Name(), text, len)) {
			char * tmp = (char*)malloc(strlen(e.Name())+1);
			strcpy(tmp, e.Name());
			e++;
			return tmp;
		}
		e++;
	}	
	return (char*)NULL;
}

char * cpl_pkg_i(const char * text, int state)
{
	static int len;
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	if (! state) {
		Cache = new pkgCache(m);
		e = Cache->PkgBegin();
		len = strlen(text);
		i = 0;
	}

	//while (i < hm) {
	while (e.end() == false) {
		//i++;
		pkgCache::Package * ppk = (pkgCache::Package *)e;
		if (ppk->CurrentState != 6) {
			e++;
			continue;
		}
		if (! strncmp(e.Name(), text, len)) {
			char * tmp = (char*)malloc(strlen(e.Name())+1);
			strcpy(tmp, e.Name());
			e++;
			return tmp;
		}
		e++;
	}	
	return (char*)NULL;
}

/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* commands completion */
char * cpl_main(const char * text_orig, int state)
{
	static int index, len;
	char * name, * tmp;
	
	char * text;
	if (text_orig[0] == ';') {
		text = (char*)text_orig+1;
	} else {
		text = (char*)text_orig;
	}
	

	if (!state) {
		index = 0;
		len = strlen(text);
	}
	while (index < CMD_NUM) {
		name = cmds[index].name;
		index++;
		if (! strncmp(text, name, len)) {
			tmp = (char*)malloc(strlen(name)+1);
			strcpy(tmp, name);
			return tmp;
		}
	}
	return (char*)NULL;
}

enum completion check_command()
{
	char * line = trimleft(rl_line_buffer);
	if (line[0] == ';') {
		line++;
		line = trimleft(line);
	}
	char * to_check = trimleft(first_word(line));
	int i = 0;
	for (; i < CMD_NUM; i++) {
		if (! strcmp(to_check, cmds[i].name)) {
			break;
		}
	}
	free(to_check);
	return cmds[i].cpl;
}

/* text completing function, its pointer is in readline's rl_attempted_completion_function variable */
char ** completion(const char * text, int start, int end)
{
	char ** m = (char**)NULL;
	if (rl_line_buffer[0] == '.') {
		return m;
	}
	char * tmp = trimleft(rl_line_buffer);
	int diff = tmp - rl_line_buffer;
	//rl_delete_text(0, (tmp-rl_line_buffer));
	if (tmp[0] == ';') {
		char toobad = 0;
		for (int i = 1; i < (start-diff); i++) {
			if ((tmp[i] != ' ')&&(tmp[i] != '\0')) {
				toobad = 1;
				break;
			}
		}
		if (! toobad) {
			m = rl_completion_matches(text, cpl_main);
			return m;
		}
	}

	if (trimleft(rl_line_buffer) == (rl_line_buffer+start)) {
	//if (start == 0) {
	//if ((start == 0) || ( (rl_line_buffer[0]==';')&& ((start==1)||(start==2) ) )) {
		m = rl_completion_matches(text, cpl_main);
	}else {
		switch (check_command()) {
			case AVAILABLE : m = rl_completion_matches(text, cpl_pkg); break;
			case INSTALLED : m = rl_completion_matches(text, cpl_pkg_i); break;
			default: break;
		}
	}
	return m;
}

/* initializes the GNU readline library */
void initialize_rl()
{
	rl_readline_name = "aptsh";
	rl_attempted_completion_function = completion;
}

struct option arg_opts[] =
{
	{"config-file", required_argument, 0, 'c' }
};

bool package_exists(char * name, enum completion ex = AVAILABLE)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;

	Cache = new pkgCache(m);
	e = Cache->PkgBegin();

	while (e.end() == false) {
		if (ex == INSTALLED) {
			pkgCache::Package * ppk = (pkgCache::Package *)e;
			if (ppk->CurrentState == 6) {
				if (! strcmp(e.Name(), name)) {
					return true;
				}
			}else {
				e++;
				continue;
			}
		}
		if (! strcmp(e.Name(), name)) {
			//e++;
			return true;
		}
		e++;
	}
	return false;
}

int validate(char * cmd)
{
	cmd = trimleft(cmd);
	if (cmd[0] == '.')
		return 0;
	char * tmp = first_word(cmd);
	char ok = 0;
	enum completion sort;
	for (int i = 0; i < CMD_NUM; i++) {
		if (! strcmp(tmp, cmds[i].name)) {
			ok = 1;
			sort = cmds[i].cpl;
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

	if (sort == AVAILABLE) {
		while (1) {
			free(tmp);
			cmd2 = trimleft(cmd2);
			tmp = first_word(cmd2);
			if (! strcmp(tmp, ""))
				break;
			//printf("P: %s -\n", tmp);

			if (! package_exists(tmp)) {
				fprintf(stderr, "Warning: Package doesn't exist: %s\n", tmp);
			}
			cmd2 = cmd2+strlen(tmp);
		}
	}

	if (sort == INSTALLED) {
		while (1) {
			free(tmp);
			cmd2 = trimleft(cmd2);
			tmp = first_word(cmd2);
			if (! strcmp(tmp, ""))
				break;

			if (! package_exists(tmp, INSTALLED)) {
				fprintf(stderr, "Warning: Package is not installed: %s\n", tmp);
			}
			cmd2 = cmd2+strlen(tmp);
		}
	}

	free(tmp);
	return 0;
}

int main(int argc, char ** argv)
{
	int c;
	int option_index = 0;
	char * execmd = NULL;
	int i;
	char help = 0;
	char * line;

	commitlog = NULL;
	first = NULL;
	//cl_size = 0;

	cfg_defaults();
	config_file = NULL;
	while ((c = getopt_long(argc, argv, "c:", arg_opts, &option_index)) != -1) {
		switch (c) {
			case 'c':
				config_file = optarg;
				cfg_parse();
				break;
		}
	}
	if (config_file == NULL) {
		config_file = CONFIG_FILE;
		cfg_parse();
	}
	initialize_rl();

	if (CFG_HISTORY_COUNT  && CFG_USE_HISTORY) {
		printf("Reading commands history...\n");
		history_truncate_file(CFG_HISTORY_FILE, CFG_HISTORY_COUNT);
		read_history_range(CFG_HISTORY_FILE, 0, CFG_HISTORY_COUNT);
	}

	printf("Reading package database...\n");
	//read_index1();
	//read_index2();
	read_indexes();
	printf("Ready.\n");
	
	for (;;) {
		help = 0;
		line = readline(CFG_PS1); /* options[0] contains ps1 from configuration file */
		
		if (CFG_USE_HISTORY)
			if (line && strcmp("", line)) {
				add_history(line);
				if (CFG_HISTORY_COUNT)
					if ((access(CFG_HISTORY_FILE, F_OK) == -1))
						write_history(CFG_HISTORY_FILE);
					else
						append_history(1, CFG_HISTORY_FILE);
			}
		
		if (line[0] == ';') {
			if (first == NULL) {
				commitlog = (struct commit_item*)malloc(sizeof(struct commit_item));
				first = commitlog;
			}else {
				commit_item * tmp = commitlog;
				commitlog = (struct commit_item*)malloc(sizeof(struct commit_item));
				tmp->next = commitlog;
			}
			commitlog->next = NULL;

			validate(line+1);
			//                     line contains ' sign, so we don't need to allocate +1 bytes.
			commitlog->text = (char*)malloc(strlen(line));
			strcpy(commitlog->text, line+1);
			continue;
		}

		if (line[0] == '.') {
			system((char*)(line+1));
			continue;
		}
	
		execmd = first_word(trimleft(line));
		
		if ((! strcmp(execmd, "quit")) || (!strcmp(execmd, "exit")) || (!strcmp(execmd, "bye")))
			break;
		
		aptcmd = line;

		for (i = 0; i < CMD_NUM; i++) {
			if (! strcmp(execmd, cmds[i].name)) {
				if (cmds[i].funct != NULL)
					(*(cmds[i].funct))();
				help = 1;
				break;
			}
		}

		if (!help) {
			printf("No such command! See 'help'.\n");
		}

		free(line);
		free(execmd);
	}
	return 0;
}


