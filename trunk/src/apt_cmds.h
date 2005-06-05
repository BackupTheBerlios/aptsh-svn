/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define CMD_NUM 42

extern char * aptcmd;

extern char use_realcmd;
extern char * realcmd;

int execute(char * line, char addhistory = 1);

/* aptsh */
int apt_dump_cfg();
int apt_dpkg();
int apt_regex();
int apt_ls();
int apt_help();
int apt_whichpkg();
int apt_listfiles();
int apt_orphans();
int apt_orphans_all();
int apt_commit();
int apt_commit_say();
int apt_commit_clear();
int apt_commit_remove();
int apt_commit_status();

/* apt-get functions */
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

/* apt-cache functions */
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



