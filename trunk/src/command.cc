#include <string>
#include <vector>
#include <list>
#include <iostream>

#include "config.h"

using namespace std;

#include <cstdlib>
#include <cstdio>

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

#include "string_utils.h"
#include "read_index.h"
#include "completions.h"
#include "column.h"
#include "config_parse.h"

#include "command.h"

command_vector commands;

string command::command_answer = "";

void command::set_answer(string answer)
{
	command_answer = answer;
}

int command::validate(char *args)
{
	return 0;
}

void command::refresh_completion()
{
}

command::command()
{
	completion = cpl_none;
	help_text = "undocumented, sorry";
}

command::~command()
{
}





command *command_vector::locate_by_name(string name)
{
	for (iterator i = begin(); i != end(); i++) {
		if ((*i)->name == name) {
			return *i;
		}
	}

	return NULL;
}




cmd_aptize::cmd_aptize(string name, string sh_command, compl_funct_type completion, validations validation, string help_text)
: sh_command(sh_command)
{
	this->name = name;
	this->completion = completion;
	this->validation = validation;
	if (help_text != "")
		this->help_text = help_text;

	has_slaves = false;
	master = NULL;
}

cmd_aptize::cmd_aptize(string name, string sh_command, vector<string> &aliases, compl_funct_type completion, validations validation, string help_text)
: sh_command(sh_command)
{
	this->aliases = aliases;
	cmd_aptize(name, sh_command, completion, validation, help_text);
}

cmd_aptize::~cmd_aptize()
{
}

int cmd_aptize::execute(char *args)
{
	string cmd = sh_command + " " + name + " " + args;
	if (command_answer == "") {
		int ret = system(cmd.c_str());
		return WEXITSTATUS(ret);
	} else {
		FILE *fp = popen(cmd.c_str(), "w");
		while ((fputs(command_answer.c_str(), fp)) != EOF);
		return pclose(fp);
	}
}

int cmd_aptize::validate(char *args)
{
	if (validation == NONE) {
		return 0;
	}

	char *pkg_name = NULL;
	word_iterator i(args);
	bool clean = true;

	/* Without the double parentheses, compiler shows ugly warning. */
	while ((pkg_name = ++i)) {
		if (validation == ALL) {
			if (! package_exists(pkg_name)) {
				fprintf(stderr, "Warning, package %s doesn't exist!\n", pkg_name);
				clean = false;
			}
		} else {
			if (! package_installed(pkg_name)) {
				fprintf(stderr, "Warning, package %s is not installed!\n", pkg_name);
				clean = false;
			}
		}

		free(pkg_name);
	}

	return !clean;
}




cmd_systemize::cmd_systemize(string name, string sh_cmd, bool ignore_args, command *master, bool has_slaves)
: sh_cmd(sh_cmd), ignore_args(ignore_args)
{
	this->name = name;
	this->master = master;
	this->has_slaves = has_slaves;
}

cmd_systemize::cmd_systemize(string name, string sh_cmd, string help_text)
{
	this->name = name;
	this->sh_cmd = sh_cmd;
	this->help_text = help_text;
	master = NULL;
	has_slaves = false;
}

cmd_systemize::cmd_systemize(string name, string sh_cmd, compl_funct_type completion,  bool ignore_args, command *master, bool has_slaves)
: sh_cmd(sh_cmd), ignore_args(ignore_args)
{
	this->name = name;
	this->completion = completion;
	this->master = master;
	this->has_slaves = has_slaves;
}

int cmd_systemize::execute(char *args)
{
	string cmd;
	if (ignore_args) {
		cmd = sh_cmd;
	} else {
		cmd = sh_cmd + " " + args;
	}
	
	int ret = system(cmd.c_str());
	return WEXITSTATUS(ret);
}




cmd_whatis::cmd_whatis()
{
	name = "whatis";
	has_slaves = false;
	master = NULL;
	completion = cpl_pkg;
	help_text = "One line description of packages";
}

int cmd_whatis::execute(char *args)
{
//	string cmd = string("apt-cache show ") + args + " | grep ^Description | cut -d\\: -f 2 | head -n 1";
	string cmd = string("apt-cache show ") + args + " | grep '^\\(Package\\|Description\\)' | cut -d\\: -f 2 | awk 'BEGIN{z=1}{if (z) {printf(\"%s:\", $_); z = 0; }else{printf(\"%s\\n\", $_); z = 1;}  }' | uniq | cut --complement -c 1";
	return system(cmd.c_str());
}




cmd_orphans::cmd_orphans()
{
	name = "orphans";
	has_slaves = true;
	master = NULL;
	completion = cpl_none;
	help_text = "List libraries not required by any installed package";
}

/* hmm, looks quite messy */
int cmd_orphans::execute(char *args)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	char *tmp = (char*)malloc(strlen(args)+strlen(SHARED_DIR)+strlen("aptsh_printer ")+4);
	sprintf(tmp, "\%s%s%s%c", SHARED_DIR, "aptsh_printer ", args, '\0');
	FILE *pipe = popen(tmp, "w");

	while (e.end() == false) {
		if (e->CurrentVer != 0) {
			const char * section = e.Section();
			if (section == NULL) /* It can't be library, since it doesn't belong to any section. */
				continue;
			if (strstr(section, "libs") || strstr(section, "libdevel")) {
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
		e++;
	}

	pclose(pipe);
	free(tmp);
	
	return 0;

}




cmd_orphans_all::cmd_orphans_all(command *master)
{
	name = "orphans-all";
	has_slaves = false;
	this->master = master;
	help_text = "List packages not required by any other installed package";
}

int cmd_orphans_all::execute(char *args)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	Cache = new pkgCache(m);
	e = Cache->PkgBegin();
	i = 0;

	char *tmp = (char*)malloc(strlen(args)+strlen(SHARED_DIR)+strlen("aptsh_printer ")+4);
	sprintf(tmp, "\%s%s%s%c", SHARED_DIR, "aptsh_printer ", args, '\0');
	
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




cmd_toupgrade::cmd_toupgrade()
{
	name = "toupgrade";
	has_slaves = false;
	master = NULL;
	help_text = "List packages with newer versions available for upgrading";
}

int cmd_toupgrade::execute(char *args)
{
	vector<pkg_to_upgrade*> *pkgs = get_to_upgrade();
	column_display *view = new column_display(3, ' ');
	view->add("NAME", 0);
	view->add("INSTALLED VER.", 1);
	view->add("AVAILABLE VER.", 2);
	view->add("", 0);
	view->add("", 1);
	view->add("", 2);

	for (vector<pkg_to_upgrade*>::iterator i = pkgs->begin(); i != pkgs->end(); i++) {
		view->add((*i)->name, 0);
		view->add((*i)->inst_ver, 1);
		view->add((*i)->avail_ver, 2);
	}

	view->dump();

	delete view;
	pkgs->clear();

	return 0;
}




cmd_news::cmd_news()
{
	name = "news";
	has_slaves = false;
	master = NULL;
	completion = cpl_pkg;
	help_text = "Obtain the latest news about the package";
}

int cmd_news::execute(char *args)
{
	if (strlen(args)) {
		word_iterator i(args);
		char *pname;
		while ((pname = ++i)) {
			int ret = system("which wget >/dev/null");
			if (WEXITSTATUS(ret) > 0) {
				fprintf(stderr, "This command can't work without wget installed (use this to install: aptsh install wget)\n");
				return 2;
			} else {
				ret = system(string(string("wget --timeout=60 --output-document=- http://packages.debian.org/changelog:")+pname+" 2>/dev/null | awk 'NR==1{print;next} /^[^ ]/{exit}{print;next}'").c_str());
				if (WEXITSTATUS(ret) > 0) {
					fprintf(stderr, "Some error occured during downloading and processing the changelog. Stopping.\n");
					return WEXITSTATUS(ret);
				}
			}
		}
	} else {
		fprintf(stderr, "Not enough parameters. You must give at least one package name.\n");
		return 1;
	}

	return 0;
}




cmd_ls::cmd_ls()
{
	name = "ls";
	help_text = "List packages matching given wildcard";
	completion = cpl_pkg;
	has_slaves = false;
	master = NULL;
}

int cmd_ls::execute(char *args)
{
	string cmd = string(escape_params((string(SHARED_DIR) + "aptsh_ls " + args).c_str()));
	FILE * pipe;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	pipe = popen(cmd.c_str(), "w");

	while (e.end() == false) {
		fprintf(pipe, "%s\n", e.Name());
		e++;
	}

	fprintf(pipe, "-\n");
	pclose(pipe);

	return 0;
}




cmd_rls::cmd_rls()
{
	name = "rls";
	help_text = "List packages matching given regular expression";
	completion = cpl_pkg;
	has_slaves = false;
	master = NULL;
}

int cmd_rls::execute(char *args)
{
	string cmd = string(escape_params((string(SHARED_DIR) + "aptsh_rls " + args).c_str()));
	FILE * pipe;

	pkgCache Cache(m);
	pkgCache::PkgIterator e = Cache.PkgBegin();

	pipe = popen(cmd.c_str(), "w");

	while (e.end() == false) {
		fprintf(pipe, "%s\n", e.Name());
		e++;
	}

	fprintf(pipe, "-\n");
	pclose(pipe);

	return 0;
}





cmd_dump_cfg::cmd_dump_cfg()
{
	name = "dump-cfg";
	has_slaves = false;
	master = NULL;
}

int cmd_dump_cfg::execute(char *args)
{
	cfg_dump();
	return 0;
}




cmd_help::cmd_help()
{
	name = "help";
	has_slaves = true;
	master = NULL;
	help_text = "Display short info about given commands, or Aptsh's manpage (if no arguments)";
	completion = cpl_main;
}

int cmd_help::execute(char *args)
{
	if (strlen(args)) {
		word_iterator i(args);
		char *cmd_name;
		command *cmd;

		column_display display(2, ' ');

		/* Double parentheses due to gcc warning when compiling with wall */
		while ((cmd_name = ++i)) {
			if ((cmd = commands.locate_by_name(string(cmd_name)))) {
				display.add(cmd_name, 0);
				display.add(cmd->help_text, 1);
				//cout << cmd_name << ": " << cmd->help_text << endl;
			} else {
				cerr << "No such command: " << cmd_name << endl;
			}
		}

		display.dump();
	} else {
		int ret = system("man aptsh");
		return WEXITSTATUS(ret);
	}

	return 0;
}




cmd_quit::cmd_quit()
{
	name = "quit";
	has_slaves = false;
	master = NULL;
}

int cmd_quit::execute(char *args)
{
	return 0;
}

