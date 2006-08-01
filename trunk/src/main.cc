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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>

using namespace std;

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

#include <readline/readline.h>
#include <readline/history.h>

#include "read_index.h"
#include "apt_cmds.h"
#include "string_utils.h"
#include "config_parse.h"
#include "dpkg_complete.h"
#include "command.h"
#include "command_queue.h"
#include "completions.h"

extern bool command_queue_mode;

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


/* Text completing function, its pointer is in readline's rl_attempted_completion_function. */
char ** completion(const char * text, int start, int end)
{
	char ** m = (char**)NULL;
	if (rl_line_buffer[0] == '.') {
		return m;
	}
	char * tmp = trimleft(rl_line_buffer);
	int diff = tmp - rl_line_buffer;

	char * tmpword;
	dpkg_complete * dpkg;
	
	/* Check if we're completing the first word, if so then use the cpl_main completion
	 * to complete commands available in Aptsh. Otherwise, check which completion we
	 * should use.
	 */
	if (trimleft(rl_line_buffer) == (rl_line_buffer+start)) {
		m = rl_completion_matches(text, cpl_main);
	} else {
		char *typed_command = trimleft(first_word(trimleft(rl_line_buffer)));
		command *found = commands.locate_by_name(typed_command);
		if (found) {
			found->refresh_completion();
			m = rl_completion_matches(text, found->completion);
		}
	}
	
	return m;
}


/* Initializes the GNU readline library. */
void initialize_rl()
{
	rl_readline_name = "aptsh";
	rl_attempted_completion_function = completion;
}


struct option arg_opts[] =
{
	{"help", no_argument, 0, '?' },
	{"command_queue_mode", no_argument, 0, 's' },
	{"version", no_argument, 0, 'v' },
	{"config-file", required_argument, 0, 'c' },
	{"execute", required_argument, 0, 'x' },
	{0, 0, 0, 0}
};


static void user_abort(int ignore)
{
	puts("bye!");
	exit(0);
}


static void libapt(bool be_silent = false)
{
	/* Initialize libapt-pkg. */
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

	command_queue_mode = false;

	use_realcmd = 0;

	/* apt-get commands */
	commands.push_back(new cmd_aptize("install", "apt-get", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("update", "apt-get", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("upgrade", "apt-get", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("dselect-upgrade", "apt-get", cpl_pkg, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("dist-upgrade", "apt-get", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("remove", "apt-get", cpl_pkg_i, cmd_aptize::INSTALLED));
	commands.push_back(new cmd_aptize("source", "apt-get", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("build-dep", "apt-get", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("check", "apt-get", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("clean", "apt-get", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("autoclean", "apt-get", cpl_none, cmd_aptize::NONE));

	/* apt-cache commands */
	commands.push_back(new cmd_aptize("show", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("dump", "apt-cache", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("add", "apt-cache", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("showpkg", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("stats", "apt-cache", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("showsrc", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("dumpavail", "apt-cache", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("unmet", "apt-cache", cpl_pkg, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("search", "apt-cache", cpl_pkg, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("depends", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("rdepends", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("pkgnames", "apt-cache", cpl_none, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("dotty", "apt-cache", cpl_pkg, cmd_aptize::ALL));
	commands.push_back(new cmd_aptize("policy", "apt-cache", cpl_pkg, cmd_aptize::NONE));
	commands.push_back(new cmd_aptize("madison", "apt-cache", cpl_pkg, cmd_aptize::ALL));

	commands.push_back(new cmd_systemize("help", "man aptsh", true, NULL, true));
	commands.push_back(new cmd_systemize("help-howto", "zcat /usr/share/doc/aptsh/HOWTO.gz", false, commands.back()));
	commands.push_back(new cmd_whatis());
	commands.push_back(new cmd_orphans());
	commands.push_back(new cmd_orphans_all(commands.back()));
	commands.push_back(new cmd_ls());
	commands.push_back(new cmd_rls());
	commands.push_back(new cmd_dpkg());
	commands.push_back(new cmd_systemize("dpkg-reconfigure", "dpkg-reconfigure", cpl_pkg_i, false, commands.back()));
	commands.push_back(new cmd_dump_cfg());

	commands.push_back(new cmd_queue());
	commands.push_back(new cmd_queue_remove(commands.back()));
	commands.push_back(new cmd_queue_clear(commands.back()->master));
	commands.push_back(new cmd_queue_commit(commands.back()->master));


	/* Handle ctrl+c. */
//	signal(SIGINT, user_abort);
	
	cfg_defaults();
	config_file = NULL;
	while ((c = getopt_long(argc, argv, "?svc:x:", arg_opts, &option_index)) != -1) {
		switch (c) {
			case 'c':
				config_file = optarg;
				cfg_parse();
				break;
			case 's':
				command_queue_mode = true;
				break;
			case 'v':
				puts(VERSION);
				return 0;
			case 'x':
#if 0
				/* We need to set both CFG_REFRESH_INDEXES and
				 * CFG_REFRESH_INDEXES_ALL to 0, because we haven't
				 * made libapt-pkg initialization yet, so it will
				 * crush when refreshing the indexes.
				 */
				
				/* Since CFG_REFRESH_INDEXES is (int)options[3].value
				 * we can't use the macro, and that's why we address
				 * it directly.
				 */
				options[3].value = 0;
				
				/* Read comment above.
				 * This is CFG_REFRESH_INDEXES_ALL
				 */
				options[4].value = 0;
#endif

				/* Initialize libapt-pkg and don't display any additional info. */
				libapt(true);
				
				execute(optarg, 0);
				return 0;
			case '?':
				/* --help and not recognized arguments should go here */
				puts("Usage: aptsh [OPTION]...\n"
				     "\n"
				     "-c, --config-file FILE   Use config file different than /etc/aptsh.conf\n"
				     "-s, --command_queue_mode            Run in queue mode\n"
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

	/* Initialize libreadline. */
	initialize_rl();

	if (getuid() > 0)
		fprintf(stderr, "Warning: Aptsh is not running as root!\n");
	
	if (CFG_HISTORY_COUNT  && CFG_USE_HISTORY) {
		printf("Reading commands history...\n");
		history_truncate_file(CFG_HISTORY_FILE, CFG_HISTORY_COUNT);
		read_history_range(CFG_HISTORY_FILE, 0, CFG_HISTORY_COUNT);
	}

	/* Initialize libapt-pkg. */
	libapt();

	i_setsig();
	
	for (;;) {
		if (command_queue_mode)
			line = readline(CFG_PS1_CQ_MODE);
		else
			line = readline(CFG_PS1); /* options[0] contains ps1 from configuration file */
		
		if (line == NULL)
			/* Probably user pressed ctrl+d. */
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


