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
#include <unistd.h>

#include "config.h"
#include "config_parse.h"
#include "string_utils.h"

char * config_file;

const char *defaults[] = {
	"apt> ", /* ps1 */
	(void*) 1, /* use_history */
	"/var/cache/apt/pkgcache.bin", /* update_file */
	(void*) 0, /* refresh_indexes */
	(void*) 1, /* refresh_indexes_all */
	"/tmp/.aptsh_history", /* history_file */     /* <- Default could be dangerous! */
	(void*) 0, /* history_count */
	"* apt> ", /* ps1_storing */
	(void*) 1, /* queue_simulate */
};

struct config_option options[] = {
#define INT 1
#define STR 0
        { "ps1", NULL, STR },
        { "use_history", NULL, INT },
        { "update_file", NULL, STR },
        { "refresh_indexes", NULL, INT },
        { "refresh_indexes_all", NULL, INT },
	{ "history_file", NULL, STR },
	{ "history_count", NULL, INT },
	{ "ps1_s", NULL, STR },
	{ "queue_simulate", NULL, INT },
};

#if 0
void * get_cfg_opt(int index) {
	if (index < OPTIONS_COUNT)
		return options[index].value;
	return NULL;
}
#endif

void cfg_dump()
{
	int i = 0;
	for (; i < OPTIONS_COUNT; i++) {
		if (options[i].is_int) {
			printf("%s = %lx\n", options[i].name, (unsigned long int)options[i].value);
		} else {
			printf("%s = %s\n", options[i].name, (char*)options[i].value);
		}
	}
}

void cfg_defaults()
{
	int i = 0;
	for (; i < OPTIONS_COUNT; i++) {
		if (options[i].is_int) {
			options[i].value = (void*)defaults[i];
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
				options[i].value = (void*)atol(value);
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


