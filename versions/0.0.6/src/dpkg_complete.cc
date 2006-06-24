/*
  Copyright (C) 2005-2006 Marcin R. Wrochniak <wrochniak@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <readline/readline.h>
#include <readline/tilde.h>

#include "string_utils.h"

#include "dpkg_complete.h"

enum importance {
	NOT_IMPORTANT,
	AVAILABLE,
	INSTALLED,
	FS,     /* Filesystem completion. */
	FS_DEB, /* Filesystem completion - but only files with .deb extension. */
};

enum arg_type {
	LONG,
	SHORT,
};

#define DPKG_OPTS_COUNT 41
struct dpkg_opion {
	char * name;
/* Which type of completion we should use. */
	enum importance cpl_decide;
/* Long/short dpkg argument. */
	enum arg_type type;
} dpkg_opts[] = {
/* short */
	{ "-i", FS_DEB, SHORT },
	{ "-a", NOT_IMPORTANT, SHORT },
	{ "-r", INSTALLED, SHORT },
	{ "-P", INSTALLED, SHORT },
	{ "-A", FS_DEB, SHORT },
	{ "-C", NOT_IMPORTANT, SHORT },
	{ "-Dh", NOT_IMPORTANT, SHORT },
/* long */	
	{ "--install", FS_DEB, LONG },
	{ "--unpack", FS, LONG },
	{ "--configure", INSTALLED, LONG },
	{ "--pending", NOT_IMPORTANT, LONG },
	{ "--remove", INSTALLED, LONG },
	{ "--purge", INSTALLED, LONG },
	{ "--update-avail", FS, LONG },
	{ "--merge-avail", FS, LONG },
	{ "--record-avail", FS_DEB, LONG },
	{ "--forget-old-unavail", NOT_IMPORTANT, LONG },
	{ "--clear-avail", NOT_IMPORTANT, LONG },
	{ "--audit", NOT_IMPORTANT, LONG },
	{ "--get-selections", NOT_IMPORTANT, LONG },
	{ "--yet-to-unpack", NOT_IMPORTANT, LONG },
	{ "--print-architecture", NOT_IMPORTANT, LONG },
	{ "--print-gnu-build-architecture", NOT_IMPORTANT, LONG },
	{ "--print-installation-architecture", NOT_IMPORTANT, LONG },
	{ "--compare-versions", NOT_IMPORTANT, LONG },
	{ "--command-fd", NOT_IMPORTANT, LONG },
	{ "--help", NOT_IMPORTANT, LONG },
	{ "--force-help", NOT_IMPORTANT, LONG },
	{ "--debug=help", NOT_IMPORTANT, LONG },
	{ "--license", NOT_IMPORTANT, LONG },
	{ "--version", NOT_IMPORTANT, LONG },
/* dpkg-query-actions */
	{ "-l", NOT_IMPORTANT, SHORT },
	{ "--list", NOT_IMPORTANT, LONG },

	{ "-s", NOT_IMPORTANT, SHORT },
	{ "--status", NOT_IMPORTANT, LONG },

	{ "-L", INSTALLED, SHORT },
	{ "--listfiles", INSTALLED, LONG },

	{ "-S", NOT_IMPORTANT, SHORT },
	{ "--search", NOT_IMPORTANT, LONG },

	{ "-p", INSTALLED, SHORT },
	{ "--print-avail", INSTALLED, LONG },
};

/* These are declarations of functions from main.cc.
 * They are used here to complete installed packages
 * and available packages.
 */
extern char * cpl_pkg(const char * text, int state);
extern char * cpl_pkg_i(const char * text, int state);

dpkg_complete::dpkg_complete(char * word, char * text, int index)
{
	word = trimleft(word);
	if (strstr(word, "--") == word) {
		completion = &normal_double;
	} else
	if (strchr(word, '-') == word) {
		completion = &normal;
	} else {
		word_iterator * wi = new word_iterator(text);
		char * word = NULL;
		bool decided = false;
		
		while ((word = wi->next_word()) != NULL) {
			for (int i = 0; i < DPKG_OPTS_COUNT; i++)
				if (! strcmp(word, dpkg_opts[i].name))
					switch (dpkg_opts[i].cpl_decide) {
						case AVAILABLE: completion = &cpl_pkg; decided = true; break;
						case INSTALLED: completion = &cpl_pkg_i; decided = true; break;
						case FS: completion = NULL; decided = true; break;
						case FS_DEB: completion = &fs_deb; decided = true; break;
						default: break;
					}
		}

		if (decided == false)
			completion = &normal_double;
	}
}

char * dpkg_complete::normal(const char * text, int state)
{
	static int i;
	static int len;

	if (! state) {
		i = 0;
		len = strlen(text);
	}

	while (i < DPKG_OPTS_COUNT) {
		i++;
		if (dpkg_opts[i-1].type == SHORT)
		if (! strncmp(dpkg_opts[i-1].name, text, len)) {
			return strdup(dpkg_opts[i-1].name);
		}
	}

	return NULL;
}

char * dpkg_complete::fs_deb(const char * text, int state)
{
	static DIR *dp;
	static struct dirent *dinfo;
	static char * real_name; /* This is the last part of path, for example
	                          * if text is '/a/b/c', then real_name is 'c'.
				  */
	
	static char * real_path; /* real_path is the string before real_name
	                          * (if text is '/a/b/c', then real_path is '/a/b/'.
				  */
	
	if (! state) {
		//int len = strlen(text);

		// FIXME: This can be done without 'found'.
		bool found = false;
		for (real_name = (char*)text+strlen(text)-1; real_name >= text; real_name--) {
			if (*real_name == '/') {
				real_name++;
				found = true;
				break;
			}
		}
		
		if (!found)
			real_name = (char*)text;
		
		real_path = strndup(text, strlen(text)-strlen(real_name));
		if (! strcmp(trimleft(real_path), ""))
			real_path = strdup("./");
		
		if (real_path[0] == '~') {
			char * tmp = real_path;
			real_path = tilde_expand(tmp);
			free(tmp);
		}
		
		if ((dp = opendir(real_path)) == NULL) {
			perror("Completion error, couldn't open the directory");
			return (char*)NULL;
		}

		rl_filename_completion_desired = 1;
	}

	struct stat *fdesc = (struct stat *)malloc(sizeof(struct stat));
	
	while ((dinfo = readdir(dp)) > 0) {
		char * name = dinfo->d_name;
		int len = strlen(name)-1;
	
		char * full_name = (char*)malloc(strlen(real_path)+strlen(name)+1);
		sprintf(full_name, "%s%s", real_path, name);
		stat(full_name, fdesc);
	
		/* This is ugly, but it's quite fast. */
		if ( ((len >= 4) && ((name[len] == 'B' || name[len] == 'b') &&
		    (name[len-1] == 'E' || name[len-1] == 'e') &&
		    (name[len-2] == 'D' || name[len-2] == 'd') &&
		    (name[len-3] == '.')) || S_ISDIR(fdesc->st_mode)) && name[0] != '.') {
			if (! strncmp(real_name, name, strlen(real_name)))
				if (strcmp(real_path, "./")) {
					free(fdesc);
					return full_name;
				} else {
					free(fdesc);
					free(full_name);
					return strdup(name);
				}
		}
	}

	closedir(dp);

	free(real_path);
	
	return NULL;
}

char * dpkg_complete::normal_double(const char * text, int state)
{
	static int i;
	static int len;

	if (! state) {
		i = 0;
		len = strlen(text);
	}

	while (i < DPKG_OPTS_COUNT) {
		i++;
		if (dpkg_opts[i-1].type == LONG)
		if (! strncmp(dpkg_opts[i-1].name, text, len)) {
			return strdup(dpkg_opts[i-1].name);
		}
	}

	return NULL;
}
