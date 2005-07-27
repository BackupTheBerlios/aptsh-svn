
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string.h"

#include "dpkg_complete.h"

enum importance {
	NOT_IMPORTANT,
	AVAILABLE,
	INSTALLED,
	FS,     // filesystem
	FS_DEB, // filesystem - but only .deb files
};

enum arg_type {
	LONG,
	SHORT,
};

#define DPKG_OPTS_COUNT 41
struct dpkg_opion {
	char * name;
	enum importance cpl_decide;
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

// These are declarations of functions from main.cc
// They are used here to complete installed packages
// and available packages
extern char * cpl_pkg(const char * text, int state);
extern char * cpl_pkg_i(const char * text, int state);

dpkg_complete::dpkg_complete(char * word, char * text, int index)
{
	word = trimleft(word);
	if (strstr(word, "--")) {
		completion = &normal_double;
	} else
	if (strchr(word, '-')) {
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
						case FS_DEB: completion = NULL; decided = true; break; // FIXME: function for fs_deb needs to be written!!
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
