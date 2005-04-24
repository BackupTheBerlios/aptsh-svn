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


// These variables are used to store commit log.
struct commit_item * commitlog; // last item in list
struct commit_item * first; // first item in list


// Completion of names of all packages
/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* packages completion */
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

// Completion of names of the installed packages
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
			// TODO: convert to strdup()
			char * tmp = (char*)malloc(strlen(e.Name())+1);
			strcpy(tmp, e.Name());
			e++;
			return tmp;
		}
		e++;
	}	
	return (char*)NULL;
}

struct command
{
	char * name;
	int (*funct)();
	enum completion cpl;
	char do_validation;
} extern cmds[];

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
			//tmp = (char*)malloc(strlen(name)+1);
			//strcpy(tmp, name);
			//return tmp;
			return strdup(name);
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

	// check if we're completing first word
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

// Number of steps
extern int commit_count;
extern char ** commitz;
extern char storing;

int main(int argc, char ** argv)
{
	int c;
	int option_index = 0;
	char * execmd = NULL;
	int i;
	//char help = 0;
	char * line;

	commitlog = NULL;
	first = NULL;
	
	commit_count = 0;
	commitz = NULL;
	storing = 0;
	use_realcmd = 0;
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
		//help = 0;
		if (storing)
			line = readline(CFG_PS1_STORING);
		else
			line = readline(CFG_PS1); /* options[0] contains ps1 from configuration file */
		
		if (! strcmp(trimleft(line), "")) {
			free(line);
			continue;
		}

		if (execute(line))
			break;

		free(line);

	}
	return 0;
}


