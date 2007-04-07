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

#include "main.h"
#include "read_index.h"
#include "string_utils.h"
#include "config_parse.h"
#include "command.h"
#include "command_queue.h"
#include "completions.h"
#include "full_completion.h"

bool command_queue_mode;


/* Returns >0 when user wants to exit. */
int execute_line(char *line, char addhistory)
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
		char *line_t = trimleft(line);
		char *fword = first_word(line_t);
		
		/* TODO: Simulations don't work now!
		 * We should do something with this.
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
		} */

		if (command *cmd = commands.locate_by_name(string(fword)))
			static_cast<cmd_aptize*>(cmd)->validate(trimleft(line_t + strlen(fword)));
		
		queue_base::add(*(new string(line_t)));
		return 0;
	}

	if (line[0] == '.') {
		system(line+1);
		
		return 0;
	}

	char * execmd = first_word(trimleft(line));
	
	if ((! strcmp(execmd, "quit")) || (!strcmp(execmd, "exit")) || (!strcmp(execmd, "bye")))
		return 1;
		
	
	char *actual_command = line;
	bool help = false;

	command *found = commands.locate_by_name(execmd);
	if (found) {
		static_cast<cmd_aptize*>(found)->execute(trimleft(actual_command+strlen(execmd)));
		return 0;
	}
	if (!help) {
		fprintf(stderr, "No such command! See 'help'.\n");
	}

	free(execmd);
	
	return 0;
}

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
char **completion(const char * text, int start, int end)
{
	char ** m = (char**)NULL;
	if (rl_line_buffer[0] == '.') {
		return m;
	}

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
	char *line;

	command_queue_mode = false;

	/* apt-get commands */
	commands.push_back(new cmd_aptize("install", "apt-get", cpl_pkg, cmd_aptize::ALL, "Install (or upgrade) one or more packages or .deb files"));
	commands.push_back(new cmd_aptize("update", "apt-get", cpl_none, cmd_aptize::NONE, "Update the list of down-loadable packages"));
	commands.push_back(new cmd_aptize("upgrade", "apt-get", cpl_none, cmd_aptize::NONE, "Upgrade all of the installed packages or just those listed"));
	commands.push_back(new cmd_aptize("dselect-upgrade", "apt-get", cpl_pkg, cmd_aptize::NONE, "Follow dselect selections (see man apt-get for details)"));
	commands.push_back(new cmd_aptize("dist-upgrade", "apt-get", cpl_none, cmd_aptize::NONE, "Upgrade to new distribution (installed and new rqd packages)"));
	commands.push_back(new cmd_aptize("remove", "apt-get", cpl_pkg_i, cmd_aptize::INSTALLED, "Remove one or more packages (see also purge)"));
	commands.push_back(new cmd_aptize("source", "apt-get", cpl_pkg, cmd_aptize::ALL, "Retrieve and unpack sources for the named packages"));
	commands.push_back(new cmd_aptize("build-dep", "apt-get", cpl_pkg, cmd_aptize::ALL, "Retrieve packages required to build listed packages"));
	commands.push_back(new cmd_aptize("check", "apt-get", cpl_none, cmd_aptize::NONE, "Update the package cache and check for broken dependencies"));
	commands.push_back(new cmd_aptize("clean", "apt-get", cpl_none, cmd_aptize::NONE, "Remove all deb files from the download cache"));
	commands.push_back(new cmd_aptize("autoclean", "apt-get", cpl_none, cmd_aptize::NONE, "Remove superseded deb files from the download cache"));

	/* apt-cache commands */
	commands.push_back(new cmd_aptize("show", "apt-cache", cpl_pkg, cmd_aptize::ALL, "Provide a detailed description of package"));
	commands.push_back(new cmd_aptize("dump", "apt-cache", cpl_none, cmd_aptize::NONE, "Dump details of all available packages"));
	commands.push_back(new cmd_aptize("add", "apt-cache", cpl_none, cmd_aptize::NONE, "Add a package file to the source cache"));
	commands.push_back(new cmd_aptize("showpkg", "apt-cache", cpl_pkg, cmd_aptize::ALL, "Show some general information for a single package"));
	commands.push_back(new cmd_aptize("stats", "apt-cache", cpl_none, cmd_aptize::NONE, "Show some basic statistics"));
	commands.push_back(new cmd_aptize("showsrc", "apt-cache", cpl_pkg, cmd_aptize::ALL, "Show source records"));
	commands.push_back(new cmd_aptize("dumpavail", "apt-cache", cpl_none, cmd_aptize::NONE, "Print an available file to stdout"));
	commands.push_back(new cmd_aptize("unmet", "apt-cache", cpl_pkg, cmd_aptize::NONE, "Show unmet dependencies"));
	commands.push_back(new cmd_aptize("search", "apt-cache", cpl_pkg, cmd_aptize::NONE, "Search the package list for a regex pattern"));
	commands.push_back(new cmd_aptize("depends", "apt-cache", cpl_pkg, cmd_aptize::ALL, "List of packages on which the given package depends"));
	commands.push_back(new cmd_aptize("rdepends", "apt-cache", cpl_pkg, cmd_aptize::ALL, "List of packages which depend/recommend/suggest the package"));
	commands.push_back(new cmd_aptize("pkgnames", "apt-cache", cpl_none, cmd_aptize::NONE, "List the names of all packages"));
	commands.push_back(new cmd_aptize("dotty", "apt-cache", cpl_pkg, cmd_aptize::ALL, "Generate package graphs for GraphVis"));
	commands.push_back(new cmd_aptize("policy", "apt-cache", cpl_pkg, cmd_aptize::NONE, "From preferences file show priorities/policy (available)"));
	commands.push_back(new cmd_aptize("madison", "apt-cache", cpl_pkg, cmd_aptize::ALL));

	commands.push_back(new cmd_help());
	commands.push_back(new cmd_systemize("help-howto", "zcat /usr/share/doc/aptsh/HOWTO.gz", false, commands.back()));
	commands.push_back(new cmd_systemize("whichpkg", "dpkg -S", string("Find the package that supplies the given command or file")));
	commands.push_back(new cmd_systemize("listfiles", "dpkg -L", string("List the files that are supplied by the named package")));
	commands.push_back(new cmd_whatis());
	commands.push_back(new cmd_orphans());
	commands.push_back(new cmd_orphans_all(commands.back()));
	commands.push_back(new cmd_toupgrade());
	commands.push_back(new cmd_news());
	commands.push_back(new cmd_ls());
	commands.push_back(new cmd_rls());
	commands.push_back(new cmd_dpkg());
	commands.push_back(new cmd_systemize("dpkg-reconfigure", "dpkg-reconfigure", cpl_pkg_i, false, commands.back()));
	commands.push_back(new cmd_dump_cfg());

	commands.push_back(new cmd_queue());
	commands.push_back(new cmd_queue_remove(commands.back()));
	commands.push_back(new cmd_queue_clear(commands.back()->master));
	commands.push_back(new cmd_queue_commit(commands.back()->master));
	commands.push_back(new cmd_queue_commit_say(commands.back()->master));

	commands.push_back(new cmd_quit());


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
				
				execute_line(optarg, 0);
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


	



	/* Parse direct commands */
	int i;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			break;
		}
	}

	if (i > 1) {
		libapt(true);

		command *cmd;
		if ((cmd = commands.locate_by_name(string(argv[1]))) != NULL) {
			string args = "";
			for (int j = 2; j < i; j++) {
				args += string(argv[j]) + ' ';
			}
			return cmd->execute((char*)args.c_str());
			//return 0;
		} else {
			fprintf(stderr, "No such command: %s\n", argv[1]);
			return 1;
		}
	} else {
		/* Now we're certainly starting interactive mode, so there's
		 * no need to be quiet.
		 */

		libapt(false);

		if (getuid() > 0)
			fprintf(stderr, "Warning: Aptsh is not running as root!\n");
	
	}




	if (CFG_HISTORY_COUNT  && CFG_USE_HISTORY) {
		printf("Reading commands history...\n");
		history_truncate_file(CFG_HISTORY_FILE, CFG_HISTORY_COUNT);
		read_history_range(CFG_HISTORY_FILE, 0, CFG_HISTORY_COUNT);
	}

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

		if (execute_line(line))
			break;

		free(line);

	}
	return 0;
}


