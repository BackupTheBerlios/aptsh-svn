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
#include <sys/stat.h>

#include <readline/readline.h>

#include "genindex.h"
#include "readindex.h"
#include "apt_cmds.h"
#include "string.h"
#include "config_parse.h"

#define CMD_NUM 33

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
	/* aptsh */
	{ "dpkg", apt_dpkg, FS },
	{ "dump-cfg", apt_dump_cfg, FS },
	{ "generate-index", apt_generate_index, NONE },
	{ "rls", apt_regex, AVAILABLE },
	{ "ls", apt_ls, AVAILABLE },
	{ "help", apt_help, NONE },
	{ "quit", NULL, NONE } 
};

/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* packages completion */
char * cpl_pkg(const char * text, int state)
{
#define COMPLETION(X1, X2)\
	static int index, len;\
	char * name, * tmp;\
\
	if (!state) {\
		index = 0;\
		len = strlen(text);\
	}\
	while (index < X2) {\
		name = X1[index];\
		index++;\
		if (! strncmp(text, name, len)) {\
			tmp = (char*)malloc(strlen(name)+1);\
			strcpy(tmp, name);\
			return tmp;\
		}\
	}\
	return (char)NULL;

	COMPLETION(pkgs, hm);
}

char * cpl_pkg_i(const char * text, int state)
{
	COMPLETION(pkgs_i, hm_i);
}

/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* commands completion */
char * cpl_main(const char * text, int state)
{
	static int index, len;
	char * name, * tmp;

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
	char * to_check = trimleft(first_word(rl_line_buffer));
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
	if (start == 0) {
		m = rl_completion_matches(text, cpl_main);
	}else {
		switch (check_command()) {
			case AVAILABLE : m = rl_completion_matches(text, cpl_pkg); break;
			case INSTALLED : m = rl_completion_matches(text, cpl_pkg_i); break;
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

long update_date(char * fn)
{
	struct stat st;
	stat(fn, &st);
	return st.st_mtime;
}

struct option arg_opts[] =
{
	{"config-file", required_argument, 0, 'c' }
};

int main(int argc, char ** argv)
{
	int c;
	int option_index = 0;
	int whattodo = 0;
	char * execmd = NULL;
	int i;
	char help = 0;
	char * line;
	
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

	if ((access(CFG_PKG_LIST, F_OK) == -1) || (access(CFG_PKG_COUNT, F_OK) == -1)) {
		generate_index1();
	} else
	if (update_date(CFG_UPDATE_FILE) > update_date(CFG_PKG_LIST)) {
		printf("Update file %s is newer than indexes, so...\n", CFG_UPDATE_FILE);
		generate_index1();
	}

	if ((access(CFG_PKG_LIST_INSTALLED, F_OK) == -1) || (access(CFG_PKG_COUNT_INSTALLED, F_OK) == -1)) {
		generate_index2();
	} else
	if (update_date(CFG_UPDATE_FILE_INSTALLED) > update_date(CFG_PKG_LIST_INSTALLED)) {
		printf("Update file %s is newer than indexes, so...\n", CFG_UPDATE_FILE);
		generate_index2();
	}

	
	printf("Reading package database...\n");
	read_index1(CFG_PKG_LIST, CFG_PKG_COUNT);
	read_index2(CFG_PKG_LIST_INSTALLED, CFG_PKG_COUNT_INSTALLED);
	printf("Ready.\n");
	
	for (;;) {
		help = 0;
		line = readline(CFG_PS1); /* options[0] contains ps1 from configuration file */
		
		if (CFG_USE_HISTORY)
			if (strcmp("", line))
				add_history(line);
		
		if (line[0] == '.') {
			system((char*)(line+1));
			continue;
		}
	
		execmd = first_word(trimleft(line));
		
		if (! strcmp(execmd, "quit"))
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
}
