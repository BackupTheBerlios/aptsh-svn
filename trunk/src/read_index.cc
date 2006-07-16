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

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

#include <stdio.h>
#include <stdlib.h>

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

#include "config_parse.h"
#include "read_index.h"

struct package * pkg_start;
struct package * pkg_iterator;

int hm;
int hm_i;

char ** pkgs; /* all packages */
char ** pkgs_i; /* installed packages */

MMap *m;

void free_indexes()
{
	delete m;
}

// Generate cache file (and load if requested)
int gen_indexes(bool load)
{
	pkgSourceList *SrcList = new pkgSourceList();
	SrcList->ReadMainList();

	OpProgress Prog;
	if (load)
		pkgMakeStatusCache(*SrcList, Prog, &m, true);
	else
		pkgMakeStatusCache(*SrcList, Prog);

	return 0;
}

int read_indexes()
{
	pkgCache * c = 0;
	m =0;


	m = new MMap(*new FileFd(CFG_UPDATE_FILE, FileFd::ReadOnly), MMap::Public | MMap::ReadOnly);
	pkgCache Cache(m);
	c = &Cache;
	pkgCache &si = *c;
	hm = si.Head().PackageCount;

	return 0;
}

/* Check whether a package exists. */
bool package_exists(char *name)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;

	Cache = new pkgCache(m);

	for (e = Cache->PkgBegin(); e.end() == false; e++) {
		if (! strcmp(e.Name(), name)) {
			//e++;
			return true;
		}
	}
	return false;
}

/* Check whether a package is installed. */
bool package_installed(char *name)
{
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;

	Cache = new pkgCache(m);

	for (e = Cache->PkgBegin(); e.end() == false; e++) {
		pkgCache::Package * ppk = (pkgCache::Package *)e;
		/* 6 means installed. */
		if ((ppk->CurrentState == 6) && (! strcmp(e.Name(), name))) {
			return true;
		}
	}
	return false;
}
