/*
  Copyright (C) 2005 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

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

#include "read_index.h"
#include "apt_cmds.h"
#include "string_utils.h"
#include "config_parse.h"
#include "dpkg_complete.h"

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

	while (e.end() == false) {
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

	while (e.end() == false) {
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

struct command extern cmds[];

/* it's executed until it returns NULL, returns a new name for readline completion if found any and not returned it before */
/* commands completion */
char * cpl_main(const char * text, int state)
{
	static int index, index_slaves, len;
	static bool slaves = false;
	static int found = 0;
	
	char * name;
	
	static bool alone = true;
	
	// Re-initialize, new completion needed
	if (!state) {
		index = 0;
		index_slaves = 0;
		len = strlen(text);
		slaves = false;
		found = 0;
		alone = true;

		/* Since it's the first launch of this function,
		 * it's the best place to check whether only one
		 * master command matches to our pattern - if not,
		 * then we add '*' sign after every master command
		 * with slaves.
		 */
		int _found = 0;
		for (int i = 0; i < CMD_NUM; i++) {
			if (!strncmp(text, cmds[i].name, len) && cmds[i].master == NULL) {
				_found++;
				
				// We're interested only about whether it's not a single command
				if (_found > 1) {
					alone = false;
					break;
				}
			}
		}
	}

	// Don't waste time and search for master commands only if we need master commands.
	if (! slaves)
		while (index < CMD_NUM) {
			name = cmds[index].name;
			struct command * now = &cmds[index];
			index++;
			if (! strncmp(text, name, len)) {
				// If master command already fits, then push slaves
				if (now->master != NULL) {
					if (strstr(text, now->master)) {
						return strdup(name);
					}
				} else {
					found++;
					/* If it's an alone master command with slaves (we've checked it earlier),
					 * then we display a '*' sign after it.
					 */
					if (now->has_slaves && !alone) {
						int tmp_len = strlen(name);
						char * tmp_cmd = (char*)malloc(tmp_len+2);
						strcpy(tmp_cmd, name);
						tmp_cmd[tmp_len] = '*';
						tmp_cmd[tmp_len+1] = '\0';
						return tmp_cmd;
						
					} else
						return strdup(name);
				}
			}
		}

	// If we find only one command, then we search for slave commands
	if ((found == 1) || slaves) {
		slaves = true;
		while (index_slaves < CMD_NUM) {
			name = cmds[index_slaves].name;
			//struct command * now = &cmds[index_slaves];
			index_slaves++;
			if (! strncmp(text, name, len) && cmds[index_slaves-1].master != NULL) {
				return strdup(name);
			}
		}
	}
	
	return (char*)NULL;
}

enum completion check_command()
{
	char * line = trimleft(rl_line_buffer);
#if 0
	if (line[0] == ';') {
		line++;
		line = trimleft(line);
	}
#endif
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

	char * tmpword;
	dpkg_complete * dpkg;
	
	// check if we're completing first word
	if (trimleft(rl_line_buffer) == (rl_line_buffer+start)) {
		m = rl_completion_matches(text, cpl_main);
	}else {
		switch (check_command()) {
			case AVAILABLE : m = rl_completion_matches(text, cpl_pkg); break;
			case INSTALLED : m = rl_completion_matches(text, cpl_pkg_i); break;
			case DPKG :
			     //printf("%d: %s\n", rl_point, rl_line_buffer);
			     tmpword = word_at_point(rl_line_buffer, rl_point);
			     dpkg = new dpkg_complete(tmpword, rl_line_buffer, rl_point);

			     if (dpkg->completion != NULL)
			     	m = rl_completion_matches(text, dpkg->completion); 

			     free(tmpword);
			     delete dpkg;

			     break;
			     
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
	{"help", no_argument, 0, '?' },
	{"storing", no_argument, 0, 's' },
	{"version", no_argument, 0, 'v' },
	{"config-file", required_argument, 0, 'c' },
	{"execute", required_argument, 0, 'x' },
	{0, 0, 0, 0}
};

// Number of steps
extern int commit_count;
extern char ** commitz;
extern char storing;

static void user_abort(int ignore)
{
	puts("bye!");
	exit(0);
}

static void libapt(bool be_silent = false)
{
	// Initialize libapt-pkg
	if (pkgInitConfig(*_config) == false ||
	pkgInitSystem(*_config, _system) == false)
	{
		_error->DumpErrors();
		//return 100;
		exit(100);
	}

	if (_config->FindB("APT::Cache::Generate", true)) {
		if (! be_silent)
			puts("Generating and mapping caches...");
		gen_indexes();
	} else {
		if (! be_silent)
			puts("Mapping caches...");
		read_indexes();
	}
}

int main(int argc, char ** argv)
{
	int c;
	int option_index = 0;
	char * line;

	commitlog = NULL;
	first = NULL;
	
	commit_count = 0;
	commitz = NULL;
	storing = 0;
	use_realcmd = 0;

	// Handle ctrl + c
	signal(SIGINT, user_abort);
	
	cfg_defaults();
	config_file = NULL;
	while ((c = getopt_long(argc, argv, "?svc:x:", arg_opts, &option_index)) != -1) {
		switch (c) {
			case 'c':
				config_file = optarg;
				cfg_parse();
				break;
			case 's':
				storing = 1;
				break;
			case 'v':
				puts(VERSION);
				return 0;
			case 'x':
#if 0
				// We need to set both CFG_REFRESH_INDEXES and
				// CFG_REFRESH_INDEXES_ALL to 0, because we haven't
				// made libapt-pkg initialization yet, so it will
				// crush when refreshing the indexes.
				
				// Since CFG_REFRESH_INDEXES is (int)options[3].value
				// we can't use the macro, and that's why we address
				// it directly.
				options[3].value = 0;
				
				// Read comment above.
				// This is CFG_REFRESH_INDEXES_ALL
				options[4].value = 0;
#endif

				// Initialize libapt-pkg and don't display any additional info
				libapt(true);
				
				execute(optarg, 0);
				return 0;
			case '?':
				/* --help and not recognized arguments should go here */
				puts("Usage: aptsh [OPTION]...\n"
				     "\n"
				     "-c, --config-file FILE   Use config file different than /etc/aptsh.conf\n"
				     "-s, --storing            Run in queue mode\n"
				     "-v, --version            Display version and exit\n"
				     "-x, --execute COMMAND    Execute COMMAND and exit\n"
				     "-?, --help               Display this text\n"
				     "\n"
				     "Please report bugs to bugs.debian.org\n");
				return 0;
			default:
				fputs("Hey! It shouldn't go here!", stderr);
		}
	}
	if (config_file == NULL) {
		config_file = CONFIG_FILE;
		cfg_parse();
	}

	// Initialize libreadline
	initialize_rl();

	if (getuid() > 0)
		fprintf(stderr, "Warning: Aptsh is not running as root!\n");
	
	if (CFG_HISTORY_COUNT  && CFG_USE_HISTORY) {
		printf("Reading commands history...\n");
		history_truncate_file(CFG_HISTORY_FILE, CFG_HISTORY_COUNT);
		read_history_range(CFG_HISTORY_FILE, 0, CFG_HISTORY_COUNT);
	}

	// Initialize libapt-pkg
	libapt();
	
	for (;;) {
		if (storing)
			line = readline(CFG_PS1_STORING);
		else
			line = readline(CFG_PS1); /* options[0] contains ps1 from configuration file */
		
		if (line == NULL)
			// Probably user pressed ctrl + d
			user_abort(0);

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


