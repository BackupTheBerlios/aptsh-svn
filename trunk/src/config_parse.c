/*
  Copyright (C) 2005 Marcin R. Wrochniak <vrok@intersec.pl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2 as
  published by the Free Software Foundation (see file COPYING for details).

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "config_parse.h"
#include "string.h"

char *defaults[] = {
	"apt> ", /* ps1 */
	(void*) 1, /* use_history */
	"/tmp/aptsh_list", /*pkg_list */
	"/tmp/aptsh_hm", /* pkg_count */
	"/var/cache/apt/pkgcache.bin", /* update_file */
	"/tmp/aptsh_list_installed", /* pkg_list_installed */
	"/tmp/aptsh_hm_installed", /* pkg_count_installed */
	(void*) 0, /* refresh_indexes */
	(void*) 1, /* refresh_indexes_all */
	"/var/lib/dpkg/status" /* update_file_installed */
};

struct config_option {
        char * name; /* name of option */
        void * value;
        char is_int; /* is this option an integer? if so, use value as a area for integer. else, use as char* */
} options[] = {
#define INT 1
#define STR 0
        { "ps1", NULL, STR },
        { "use_history", NULL, INT },
        { "pkg_list", NULL, STR },
        { "pkg_count", NULL, STR },
        { "update_file", NULL, STR },
        { "pkg_list_installed", NULL, STR },
        { "pkg_count_installed", NULL, STR },
        { "refresh_indexes", NULL, INT },
        { "refresh_indexes_all", NULL, INT },
        { "update_file_installed", NULL, STR }
};

void * get_cfg_opt(int index) {
	if (index < OPTIONS_COUNT)
		return options[index].value;
	return NULL;
}

void cfg_dump()
{
	int i = 0;
	for (; i < OPTIONS_COUNT; i++) {
		if (options[i].is_int) {
			printf("%s = %d\n", options[i].name, options[i].value);
		} else {
			printf("%s = %s\n", options[i].name, options[i].value);
		}
	}
}

void cfg_defaults()
{
	int i = 0;
	for (; i < OPTIONS_COUNT; i++) {
		if (options[i].is_int) {
			options[i].value = defaults[i];
		}else {
			free(options[i].value);
			options[i].value = (void*)malloc(strlen(defaults[i])+1);
			strcpy(options[i].value, defaults[i]);
		}
	}
}

int cfg_set(char * name, char * value)
{
	int i;
	for (i = 0; i < OPTIONS_COUNT; i++) {
		if (! strcmp(name, options[i].name)) {
			if (options[i].is_int) {
				options[i].value = (void*)atoi(value);
			}else {
				options[i].value = (void*)malloc(strlen(value)+1);
				strcpy(options[i].value, value);
			}
			return 0;
		}
	}
	fprintf(stderr, "No such configuration option: %s!\n", name);
	return 1;
}

int cfg_parse()
{
	FILE * fp;
	char buf[1023] = "";
	char r;
	char * trimmed = NULL;
	char tmp[1023];
	int len = 0;
	char comment = 0;
	int x = 0;
	char typing = 0;
	int i = 0;
	if (access(config_file, F_OK) == -1) {
		fprintf(stderr, "Configuration file doesn't exist!\n");
		return 1;
	}
	if (! (fp = fopen(config_file, "r"))) {
		fprintf(stderr, "Can't open configuration file!\n");
		return 1;
	}
	while ((r = fgetc(fp)) != EOF) {
		if ((r != '\n') && comment)
			continue;
		if (((r == ' ') || (r == '\t')) && !typing) {
			continue;
		}
		if (!typing && (r == '#')) {
			comment = 1;
			continue;
		}
		if (!typing)
			typing = 1;
		if (r == '\n') {
			if (! comment) {
				buf[x] = '\0';
				len = strlen(buf);
				i = 0;
				while (buf[i] != '=') { if (i < len) i++; else break; }
				strncpy(tmp, buf, i);
				tmp[i] = '\0';
				trimmed = trim(tmp);
				if (strlen(trimmed)) {
					cfg_set(trimmed, (char*)(buf+i+1));
				}
				free(trimmed);
			}
			comment = 0;
			typing = 0;
			strcpy(buf, "");
			x = 0;
			continue;
		}
		
		if (x > 1022) {
			fprintf(stderr, "config err\n");
			return 1;
		}
		buf[x++] = r;
	}

	
	fclose(fp);
	return 0;
}

/* vim: ts=4
*/

