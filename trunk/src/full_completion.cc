#include <vector>
#include <string>
using namespace std;

#include <cstdlib>
#include <cstring>

#include <readline/readline.h>
#include <readline/history.h>

#include "string_utils.h"
#include "command.h"
#include "completions.h"

#include "full_completion.h"

template<class child_name> vector<fc_option> full_completion<child_name>::options = vector<fc_option>();

fc_option::fc_option(string name_short, string name_long, compl_funct_type completion)
: name_short(name_short), name_long(name_long), completion(completion)
{
}

template<class child_name>
void full_completion<child_name>::refresh_completion()
{
	char *current_word = word_at_point(rl_line_buffer, rl_point);
	if (strstr(current_word, "--") == current_word) {
		completion = &child_name::cpl_long_opts;
	} else 
	if (strchr(current_word, '-') == current_word) {
		completion = &child_name::cpl_short_opts;
	} else {
		backward_word_iterator it(rl_line_buffer, rl_point);
		char *i;
		completion = &cpl_long_opts;
		while ((i = it.prev_word()) != NULL) {
			if (strstr(i, "--") == i) {
				bool found = false;
				for (vector<fc_option>::iterator j = options.begin(); j != options.end(); j++) {
					if (! strcmp(i, j->name_long.c_str())) {
						completion = j->completion;
						found = true;
						break;
					}
				}
				if (found) {
					free(i);
					break;
				}
			} else
			if (strchr(i, '-') == i) {
				bool found = false;
				for (vector<fc_option>::iterator j = options.begin(); j != options.end(); j++) {
					if (! strcmp(i, j->name_short.c_str())) {
						completion = j->completion;
						found = true;
						break;
					}
				}
				if (found) {
					free(i);
					break;
				}
			}

			free(i);
		}
	}
}


template<class child_name>
char *full_completion<child_name>::cpl_long_opts(const char *text, int state)
{
	static vector<fc_option>::iterator i;
	static int len;
	if (! state) {
		i = options.begin();
		len = strlen(text);
	}

	while (i != options.end()) {
		if (! strncmp(i->name_long.c_str(), text, len)) {
			string tmp(i->name_long.c_str());
			i++;
			return strdup(tmp.c_str());
		}
		i++;
	}

	return NULL;
}

template<class child_name>
char *full_completion<child_name>::cpl_short_opts(const char *text, int state)
{
	static vector<fc_option>::iterator i;
	static int len;
	if (! state) {
		i = options.begin();
		len = strlen(text);
	}

	while (i != options.end()) {
		if (! strncmp(i->name_short.c_str(), text, len)) {
			string tmp(i->name_short.c_str());
			i++;
			return strdup(tmp.c_str());
		}
		i++;
	}

	return NULL;
}




cmd_dpkg::cmd_dpkg()
{
	name = "dpkg";
	master = NULL;
	has_slaves = true;

	options.push_back(fc_option("-i", "--install", cpl_fs_deb));
	options.push_back(fc_option("-r", "--remove", cpl_pkg_i));
}

int cmd_dpkg::execute(char *args)
{
	return system(args);
}

