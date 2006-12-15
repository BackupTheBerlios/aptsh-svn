

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

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

#include "read_index.h"
#include "string_utils.h"
#include "config_parse.h"
#include "command.h"

#include "completions.h"


char *cpl_none(const char *text, int state)
{
	return (char*)NULL;
}

/* All available packages completion.
 * It's executed until it returns NULL. Returns a new name for readline
 * completion if found any, and not returned it before packages completion.
 */
char *cpl_pkg(const char *text, int state)
{
	static int len;
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	if (! state) {
		Cache = new pkgCache(m);
		e = Cache->PkgBegin();
		len = strlen(text);
		i = 0;
	}

	while (e.end() == false) {
		if (! strncmp(e.Name(), text, len)) {
			char *tmp = strdup(e.Name());
			e++;
			return tmp;
		}
		e++;
	}	
	return (char*)NULL;
}

/* Installed packages completion. */
char *cpl_pkg_i(const char * text, int state)
{
	static int len;
	static pkgCache * Cache;
	static pkgCache::PkgIterator e;
	static int i;
	
	if (! state) {
		Cache = new pkgCache(m);
		e = Cache->PkgBegin();
		len = strlen(text);
		i = 0;
	}

	while (e.end() == false) {
		pkgCache::Package * ppk = (pkgCache::Package *)e;
		if (ppk->CurrentState != 6) {
			e++;
			continue;
		}
		if (! strncmp(e.Name(), text, len)) {
			char *tmp = strdup(e.Name());
			e++;
			return tmp;
		}
		e++;
	}	
	return (char*)NULL;
}


/* It's executed until it returns NULL, returns a new name for readline
 * completion if found any and not returned it before commands completion.
 */
char *cpl_main(const char * text, int state)
{
	static vector<command*>::iterator i;
	static int len;
	typedef enum display_type {
		ALL, ROOTS, GROUP
	};
	static display_type display;
	static command *group_master;

	if (! state) {
		len = strlen(text);
		command_vector::iterator j;
		char c = 0;
		for (j = commands.begin(); j != commands.end(); j++) {
			if (!strncmp(text, (*j)->name.c_str(), len) && !(*j)->master) {
				c++;
				if (c > 1) {
					break;
				} else
				if (c == 1) {
					group_master = *j;
				}
			}
		}
		
		if (c == 0) {
			/* Maybe commands from some group fit... */
			display = ALL;
		} else
		if ((c == 1) && group_master->has_slaves) {
			/* Only one master commands fits, so display only commands from its group. */
			display = GROUP;
		} else {
			/* Display basic commands. */
			display = ROOTS;
		}

		i = commands.begin();
	}

	while (i != commands.end()) {
		bool ok = false;
		if (! strncmp(text, (*i)->name.c_str(), len)) {
			switch (display) {
				case ROOTS:
					if ((*i)->master == NULL) {
						if ((*i)->has_slaves) {
							char *tmp = strdup(string((*i)->name+"*").c_str());
							i++;
							return tmp;
						}
						ok = true;
					}
					break;
				case GROUP:
					if (((*i)->master == group_master) || (group_master == *i)) {
						ok = true;
					}
					break;
				case ALL:
					ok = true;
					break;
			}
			
			if (ok) {
				char *tmp = strdup((*i)->name.c_str());
				i++;
				return tmp;
			}
		}
		i++;
	}
	
	return (char*)NULL;
}

char *cpl_fs_deb(const char * text, int state)
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
