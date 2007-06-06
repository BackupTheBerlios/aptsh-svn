#include <list>
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
#include "read_index.h"

#include "full_completion.h"

template<class child_name> list<fc_option> full_completion<child_name>::options = list<fc_option>();

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
				for (list<fc_option>::iterator j = options.begin(); j != options.end(); j++) {
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
				for (list<fc_option>::iterator j = options.begin(); j != options.end(); j++) {
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
	static list<fc_option>::iterator i;
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
	static list<fc_option>::iterator i;
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
	help_text = "package manager for Debian";

	/* Options which modify behaviour of actions. */
	options.push_back(fc_option("-R", "--recursive", cpl_none));
	options.push_back(fc_option("-a", "--pending", cpl_none));

	/* Actions */
	options.push_back(fc_option("-i", "--install", cpl_fs_deb));
	options.push_back(fc_option("", "--unpack", cpl_fs_deb));
	options.push_back(fc_option("-A", "--record_avail", cpl_fs_deb));
	options.push_back(fc_option("", "--configure", cpl_pkg_i));
	options.push_back(fc_option("-r", "--remove", cpl_pkg_i));
	options.push_back(fc_option("-P", "--purge", cpl_pkg_i));
	options.push_back(fc_option("", "--get-selections", cpl_none));
	options.push_back(fc_option("", "--set-selections", cpl_none));
	options.push_back(fc_option("", "--clear-selections", cpl_none));
	options.push_back(fc_option("", "--update-avail", cpl_none));
	options.push_back(fc_option("", "--merge-avail", cpl_none));
	options.push_back(fc_option("", "--clear-avail", cpl_none));
	options.push_back(fc_option("", "--forget-old-unavail", cpl_none));
	options.push_back(fc_option("-s", "--status", cpl_pkg_i));
	options.push_back(fc_option("-p", "--print-avail", cpl_pkg_i));
	options.push_back(fc_option("-L", "--listfiles", cpl_pkg_i));
	options.push_back(fc_option("-l", "--list", cpl_pkg_i));
	options.push_back(fc_option("-S", "--search", cpl_none));
	options.push_back(fc_option("-C", "--audit", cpl_none));
	options.push_back(fc_option("", "--print-architecture", cpl_none));
	options.push_back(fc_option("", "--compare-versions", cpl_pkg_i));
	options.push_back(fc_option("", "--force-help", cpl_none));
	options.push_back(fc_option("-Dh", "--debug=help", cpl_none));
	options.push_back(fc_option("-h", "--help", cpl_none));
	options.push_back(fc_option("", "--version", cpl_none));
	options.push_back(fc_option("", "--license", cpl_none));
	
	/* dpkg-deb */
	options.push_back(fc_option("-b", "--build", cpl_none));
	options.push_back(fc_option("-c", "--contents", cpl_fs_deb));
	options.push_back(fc_option("-I", "--info", cpl_fs_deb));
	options.push_back(fc_option("-f", "--field", cpl_fs_deb));
	options.push_back(fc_option("-x", "--extract", cpl_fs_deb));
	options.push_back(fc_option("-X", "--vextract", cpl_fs_deb));
	options.push_back(fc_option("", "--fsys-tarfile", cpl_none));

	options.push_back(fc_option("", "--admindir=", cpl_none));
	options.push_back(fc_option("", "--root=", cpl_none));
	options.push_back(fc_option("", "--instdir=", cpl_none));
	options.push_back(fc_option("-O", "--selected-only", cpl_none));
	options.push_back(fc_option("-E", "--skip-same-version", cpl_none));
	options.push_back(fc_option("-G", "--refuse-downgrade", cpl_none));
	options.push_back(fc_option("-B", "--auto-deconfigure", cpl_none));
	options.push_back(fc_option("", "--no-debsig", cpl_none));
	options.push_back(fc_option("", "--simulate", cpl_none)); // --no-act, --dry-run
	options.push_back(fc_option("-D", "--debug=", cpl_none));
	options.push_back(fc_option("", "--status-fd", cpl_none));
	options.push_back(fc_option("", "--log=", cpl_none));
	options.push_back(fc_option("", "--ignore-depends=", cpl_none));
	options.push_back(fc_option("", "--force-", cpl_none));
	options.push_back(fc_option("", "--no-force-", cpl_none));
	options.push_back(fc_option("", "--abort-after", cpl_none));
}

int cmd_dpkg::execute(char *args)
{
	int ret = system(args);
	check_cache();
	return WEXITSTATUS(ret);
}

