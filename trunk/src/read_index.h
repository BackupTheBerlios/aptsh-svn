/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef READ_INDEX_H
#define READ_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif


extern class MMap *m;

int gen_indexes(bool load = true);

int read_indexes();

void check_cache();

bool package_exists(char *name);
bool package_installed(char *name);

struct package
{
	char * name;
	struct package * next;
};

extern struct package * pkg_start;
extern struct package * pkg_iterator;

void free_indexes();

void pkg_next(struct package * pkg);

extern int hm;
extern int hm_i;

extern char ** pkgs; /* All packages. */
extern char ** pkgs_i; /* Only installed packages. */

struct pkg_to_upgrade {
	string name, inst_ver, avail_ver;

	pkg_to_upgrade(string name, string inst_ver, string avail_ver)
	: name(name), inst_ver(inst_ver), avail_ver(avail_ver)
	{}
};

/* This list is actually a vector, uh... (it's because it needs to be sortable) */
vector<pkg_to_upgrade*> *get_to_upgrade();

#ifdef __cplusplus
}
#endif

#endif



