/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef APT_CMDS_H
#define APT_CMDS_H

#define CMD_NUM 44

enum completion {
	INSTALLED,
	AVAILABLE,
	DPKG,
	FS,
	NONE
};

struct commit_item {
	char * text;
	struct commit_item * next;
};

extern struct commit_item * commitlog;
extern struct commit_item * first;

struct command
{
	char * name;
	int (*funct)();
	enum completion cpl;
	/* Decides whether words after command sould be validated,
	 * example: "install apt dpkg kde qwertyqqw" is going to warn on "qwertyqqw",
	 * because such a package doesn't exist (do_validation for "install" is true)
	 * and it checks for existence in all packages, because cpl of "install" is AVAILABLE.
	 */
	bool do_validation;
	char * master; /* Master command (example: commit - master; commit-clear, commit-remove,
	                * commit-status - slaves). NULL if there's no master command.
			*/
	bool has_slaves;
	bool apt_get; /* If command launches the apt-get command, then we can simulate it
	               * by adding -s/--simulate option (CFG_QUEUE_SIMULATE decides whether
		       * we can or can't do the simulation).
		       */
};

extern char * aptcmd;

extern bool use_realcmd;
extern char * realcmd;

int execute(char * line, char addhistory = 1);

/* Aptsh specific commands. */
int apt_dump_cfg();
int apt_dpkg();
int apt_dpkg_reconfigure();
int apt_regex();
int apt_ls();
int apt_help();
int apt_help_howto();
int apt_whichpkg();
int apt_listfiles();
int apt_orphans();
int apt_orphans_all();
int apt_queue();
int apt_queue_commit();
int apt_queue_commit_say();
int apt_queue_clear();
int apt_queue_remove();

/* Apt-get specific. */
int apt_install();
int apt_update();
int apt_upgrade();
int apt_dselect_upgrade();
int apt_dist_upgrade();
int apt_remove();
int apt_source();
int apt_build_dep();
int apt_check();
int apt_clean();
int apt_autoclean();

/* Apt-cache specific. */
int apt_show();
int apt_dump();
int apt_add();
int apt_showpkg();
int apt_stats();
int apt_showsrc();
int apt_dumpavail();
int apt_unmet();
int apt_search();
int apt_depends();
int apt_rdepends();
int apt_pkgnames();
int apt_dotty();
int apt_policy();
int apt_madison();
int apt_whatis();

#endif

