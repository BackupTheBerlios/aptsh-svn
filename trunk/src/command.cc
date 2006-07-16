#include <string>
#include <vector>
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
#include "dpkg_complete.h"
#include "config_parse.h"

#include "command.h"

commands_vector commands;

command::command()
{
	completion = cpl_none;
}

command::~command()
{
}

command *commands_vector::locate_by_name(string name)
{
	for (iterator i = begin(); i != end(); i++) {
		if ((*i)->name == name) {
			return *i;
		}
	}

	return NULL;
}



cmd_aptize::cmd_aptize(string name, string sh_command, char* (*completion)(const char*, int), validations validation)
: sh_command(sh_command)
{
	this->name = name;
	this->completion = completion;
	this->validation = validation;

	has_slaves = false;
	master = NULL;
}

cmd_aptize::cmd_aptize(string name, string sh_command, vector<string> &aliases, char* (*completion)(const char*, int), validations validation)
: sh_command(sh_command)
{
	this->aliases = aliases;
	cmd_aptize(name, sh_command, completion, validation);
}

cmd_aptize::~cmd_aptize()
{
}

int cmd_aptize::execute(char *args)
{
	string cmd = sh_command + " " + name + " " + args;
	return system(cmd.c_str());
}

void cmd_aptize::refresh_completion()
{
	return;
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

cmd_systemize::cmd_systemize(string name, string sh_cmd, char* (*completion)(const char*, int),  bool ignore_args, command *master, bool has_slaves)
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
	
	return system(cmd.c_str());
}

int cmd_systemize::validate(char *args)
{
	return 0;
}

void cmd_systemize::refresh_completion()
{
	return;
}




cmd_whatis::cmd_whatis()
{
	name = "whatis";
	has_slaves = false;
	master = NULL;
	completion = cpl_pkg;
}

int cmd_whatis::execute(char *args)
{
//	string cmd = string("apt-cache show ") + args + " | grep ^Description | cut -d\\: -f 2 | head -n 1";
	string cmd = string("apt-cache show ") + args + " | grep '^\\(Package\\|Description\\)' | cut -d\\: -f 2 | awk 'BEGIN{z=1}{if (z) {printf(\"%s:\", $_); z = 0; }else{printf(\"%s\\n\", $_); z = 1;}  }' | uniq | cut --complement -c 1";
	return system(cmd.c_str());
}

void cmd_whatis::refresh_completion()
{
//	completion = cpl_pkg;
	return;
}

int cmd_whatis::validate(char *args)
{
	return 0;
}




cmd_orphans::cmd_orphans()
{
	name = "orphans";
	has_slaves = true;
	master = NULL;
	completion = cpl_none;
}

/* TODO: Do something with that mess below. */
int cmd_orphans::execute(char *args)
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
	//char *actual_command_t = trimleft(actual_command);
	//char *fw = first_word(actual_command_t);
	//actual_command_t += strlen(fw);
	//actual_command_t = trimleft(actual_command_t);
	char *tmp = (char*)malloc(strlen(args)+strlen(SHARED_DIR)+strlen("aptsh_printer ")+4);
	sprintf(tmp, "\%s%s%s%c", SHARED_DIR, "aptsh_printer ", args, '\0');
//	free(fw);
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

int cmd_orphans::validate(char *args)
{
	return 0;
}

void cmd_orphans::refresh_completion()
{
}




cmd_orphans_all::cmd_orphans_all(command *master)
{
	name = "orphans-all";
	has_slaves = false;
	this->master = master;
}

int cmd_orphans_all::execute(char *args)
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
#if 0
	char *actual_command_t = trimleft(actual_command);
	char *fw = first_word(actual_command_t);
	actual_command_t += strlen(fw);
	//actual_command_t = trimleft(actual_command_t);
	char *tmp = (char*)malloc(strlen(actual_command_t)+strlen(SHARED_DIR)+strlen(LIBEXEC_PREFIX)+strlen("printer ")+4);
	sprintf(tmp, "\%s%s%s%s%c", SHARED_DIR, LIBEXEC_PREFIX, "printer ", actual_command_t, '\0');
	free(fw);
#endif
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

int cmd_orphans_all::validate(char *args)
{
	return 0;
}

void cmd_orphans_all::refresh_completion()
{
}




cmd_ls::cmd_ls()
{
	name = "ls";
	completion = cpl_pkg;
}

int cmd_ls::execute(char *args)
{
	string cmd = string(SHARED_DIR) + "aptsh_ls " + args;
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

int cmd_ls::validate(char *args)
{
	return 0;
}

void cmd_ls::refresh_completion()
{
}




cmd_rls::cmd_rls()
{
	name = "rls";
	completion = cpl_pkg;
}

int cmd_rls::execute(char *args)
{
	string cmd = string(SHARED_DIR) + "aptsh_rls " + args;
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

int cmd_rls::validate(char *args)
{
	return 0;
}

void cmd_rls::refresh_completion()
{
}



cmd_dpkg::cmd_dpkg()
{
	name = "dpkg";
	master = NULL;
	has_slaves = true;
}

int cmd_dpkg::execute(char *args)
{
	return system(args);
}

int cmd_dpkg::validate(char *args)
{
	return 0;
}

void cmd_dpkg::refresh_completion()
{
	char *current_word = word_at_point(rl_line_buffer, rl_point);
	dpkg_complete *actual_completion = new dpkg_complete(current_word, rl_line_buffer, rl_point);

	if (actual_completion->completion != NULL) {
		completion = actual_completion->completion;
	}

	free(current_word);
	delete actual_completion;
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

int cmd_dump_cfg::validate(char *args)
{
	return 0;
}

void cmd_dump_cfg::refresh_completion()
{
}

