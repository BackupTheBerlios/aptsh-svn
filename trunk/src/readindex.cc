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
#include "readindex.h"

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
}


