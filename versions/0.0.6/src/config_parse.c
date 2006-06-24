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
	"* apt> ", /* ps1_cq_mode */
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
	static char hex_accepted[] = "1234567890abcdefABCDEF\0";
	char buf[1024] = "";
	char buf_v[1024] = "";
	int x = 0;
	signed int r;

	/* Stages:
	 * 0 - nothing started yet
	 * 1 - getting variable's name
	 * 2 - whitespaces before '='
	 * 3 - getting variable's value
	 */
	char stage = 0;

	if (access(config_file, F_OK) == -1) {
		fprintf(stderr, "Configuration file doesn't exist!\n");
		return 1;
	}
	if (! (fp = fopen(config_file, "r"))) {
		fprintf(stderr, "Can't open configuration file!\n");
		return 1;
	}
	while ((r = fgetc(fp)) != EOF) {
		if (r == '\n') {
			if (stage == 3) {
				buf_v[x] = '\0';
				cfg_set(buf, buf_v);
			}
			stage = 0;
			x = 0;
			continue;
		}
		
		if ((r == '#') && !stage) {
			x = 0;
			while (fgetc(fp) != '\n');
			continue;
		}
		
		if ((r == '=') && ((stage == 2) || (stage == 1))) {
			buf[x] = '\0';
			x = 0;
			stage = 3;
			continue;
		}
	
		if ((r != ' ') && (r != '\t') && (stage <= 1)) {
			stage = 1;
			if (x >= 1022) {
				fprintf(stderr, "Too long variable name.\n");
				return 1;
			}

			buf[x++] = r;
		}

		if ((r == ' ') && (r == '\t')) {
			if (stage == 2) {
				stage = 2;
			}

			if (stage != 3)
				continue;
		}

		if (stage == 3) {
			if (x >= 1022) {
				fprintf(stderr, "Too long value. Maximum is 1023.\n");
				return 1;
			}
			
			if (r == '\\') {
				if ((r = fgetc(fp)) == 'x') {
					char sign[] = "  \0";
					if (! strchr(hex_accepted, (sign[0] = fgetc(fp)))) {
						/* FIXME: we should check 'r' now. */
						continue;
					}
					if (! strchr(hex_accepted, (sign[1] = fgetc(fp)))) {
						/* FIXME: we should check 'r' now. */
						continue;
					}
					buf_v[x++] = (char)strtol(sign, (char**)NULL, 16);
					continue;
				} else
				if ((r == '\n') || (r == EOF)) {
					buf_v[x++] = '\\';
					if (stage == 3) {
						buf_v[x] = '\0';
						cfg_set(buf, buf_v);
					}
					stage = 0;
					x = 0;
					continue;				
				} else
				if (x >= 1022) {
					fprintf(stderr, "Too long value. Maximum is 1023.\n");
					return 1;
				} else {
					buf_v[x++] = '\\';
					buf_v[x++] = r;
					continue;
				}	
			}
		
			buf_v[x++] = r;
		}
			
	}
	
	fclose(fp);
	return 0;
}


