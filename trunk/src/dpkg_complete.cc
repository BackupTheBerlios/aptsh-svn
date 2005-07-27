
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

#define DPKG_OPTS_COUNT 4
struct dpkg_opion {
	char * name;
	enum importance cpl_decide;
	enum arg_type type;
} dpkg_opts[] = {
	{ "-i", FS, SHORT },
	{ "-a", AVAILABLE, SHORT },
	{ "--install", FS, LONG },
	{ "--unpack", FS, LONG },
};


/*char * dpkg_opts[] = {
	"-i",
	"-r",
	"-s",
	"-S",
};*/

dpkg_complete::dpkg_complete(char * word, char * text, int index)
{
	word = trimleft(word);
	if (strstr(word, "--")) {
		completion = &normal_double;
	} else
	if (strchr(word, '-')) {
		completion = &normal;
	} else {
		// scan
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
