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

#include <fstream>
#include <vector>
#include <string>

using namespace std;

#include <stdio.h>
#include <stdlib.h>

#include <apt-pkg/pkgcachegen.h>

#include "config_parse.h"
#include "readindex.h"

void free_index1()
{
	free(pkgs);
}

int read_index1()
{
	pkgCache * c = 0;
	MMap *m =0;
	int count = 0;
	m = new MMap(*new FileFd(CFG_UPDATE_FILE, FileFd::ReadOnly), MMap::Public | MMap::ReadOnly);
	pkgCache Cache(m);
	c = &Cache;
	pkgCache &si = *c;
	hm = si.Head().PackageCount;
	pkgs = (char**)malloc(hm*sizeof(char*));
	for (pkgCache::PkgIterator e = si.PkgBegin(); e.end() == false; e++) {
		pkgs[count] = (char*)malloc(strlen(e.Name())+1);
		strcpy(pkgs[count], e.Name());
		count++;
	}
	delete m;
}

void free_index2()
{
	struct package * pp = pkg_start;
	struct package * tmp;
	while (pp->next != NULL) {
		free(pp->name);
		tmp = pp->next;
		free(pp);
		pp = tmp;
	}
}

int read_index2()
{
	char tmp[1023];
	int i = 0;
	int x = 0;
	char c;
	bool ignore = false;
	FILE * fp;
	struct package * p;
	struct package * prev;
	if (! (fp = fopen(CFG_UPDATE_FILE_INSTALLED, "r"))) {
		fprintf(stderr, "Can't open %s file!\n", CFG_UPDATE_FILE_INSTALLED);
		return 1;
	}
	while ((c = fgetc(fp)) != EOF) {
		if (c != '\n') {
			if (ignore)
				continue;
			if (i == 1023) {
				ignore = true;
				continue;
			}
			tmp[i++] = c;
		}else {
			if (ignore) {
				ignore = false;
				i = 0;
				continue;
			}
			tmp[i] = '\0';
			if (strstr(tmp, "Package:") == tmp) {
				prev = p;
				p = (struct package*)malloc(sizeof(package));
				p->name = (char*)malloc(strlen(tmp)-8);
				p->next = NULL;
				strcpy(p->name, tmp+9);
				if (x != 0) {
					prev->next = p;
				}else {
					pkg_start = p;			
				}
				x++;
			}
			i = 0;
		}
	}
	fclose(fp);

/* Below is the method of iterating through names of packages.
 *
 * package * pp;
 *	for (pp = pkg_start; pp->next != NULL; pp = pp->next) {
 *		printf("%s\n", pp->name);
 *	}
 *	printf("%s\n", pp->name);
 */
}

/* vim: ts=4
*/

