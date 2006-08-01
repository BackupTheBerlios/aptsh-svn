#include <iostream>
#include <vector>
#include <list>
#include <string>

using namespace std;

#include <cstdio>
#include <cstring>

#include "string_utils.h"
#include "command.h"

#include "command_queue.h"

list<string> queue_base::queue = list<string>();

void queue_base::add(string &cmd_text)
{
	queue.push_back(cmd_text);
}





cmd_queue::cmd_queue()
{
	name = "queue";
	master = NULL;
	has_slaves = true;
}

int cmd_queue::execute(char *args)
{
	int j = 1;
	for (list<string>::iterator i = queue.begin(); i != queue.end(); i++) {
		printf("%d: %s\n", j++, i->c_str());
	}
	return 0;
}

int cmd_queue::validate(char *args)
{
	return 0;
}

void cmd_queue::refresh_completion()
{
}




cmd_queue_remove::cmd_queue_remove(command *master)
{
	name = "queue-remove";
	has_slaves = false;
	this->master = master;
}

int cmd_queue_remove::execute(char *args)
{
	word_iterator i(args);
	char *arg;
	while ((arg = (++i)) != NULL) {
		int x;
		if (strchr(arg, '-')) {
			size_t divider;
			string arg_s(arg);
			if ((divider = arg_s.find('-')) != string::npos) {
				string s1 = string(arg_s, 0, divider),
				       s2 = string(arg_s, divider+1, arg_s.length()-divider-1);
				int from, to;

				if (s1 == "last")
					from = queue.size();
				else
					from = atoi(s1.c_str());

				if (s2 == "last")
					to = queue.size();
				else
					to = atoi(s2.c_str());

				if ((from == 0) || (to == 0)) {
					cerr << "There's no item numbered 0.";
					return 1;
				}

				if (to < from) {
					int tmp = to;
					to = from;
					from = tmp;
				}
				
				int j = 1;
				list<string>::iterator from_i, to_i;
				for (list<string>::iterator i = queue.begin(); i != queue.end(); i++) {
					if (j == from)
						from_i = i;
					if (j == to) {
						to_i = i;
						break;
					}
					j++;
				}
				queue.erase(from_i, to_i);
				/* list<>::erase(first, last) erases the range [first, last),
				 * so we erase one more element.
				 */
				queue.erase(to_i);
			}

		} else
		if ((x = atoi(arg)) > 0) {
			int j = 1;
			for (list<string>::iterator i = queue.begin(); i != queue.end(); i++) {
				/* The element we need to remove */
				if (j == x) {
					queue.erase(i);
					break;
				}
				j++;
			}
		}
		free(arg);
	}

	return 0;
}

int cmd_queue_remove::validate(char *args)
{
	return 0;
}

void cmd_queue_remove::refresh_completion()
{
}





cmd_queue_clear::cmd_queue_clear(command *master)
{
	name = "queue-clear";
	has_slaves = false;
	this->master = master;
}

int cmd_queue_clear::execute(char *args)
{
	queue.clear();
	return 0;
}

int cmd_queue_clear::validate(char *args)
{
	return 0;
}

void cmd_queue_clear::refresh_completion()
{
	return;
}





cmd_queue_commit::cmd_queue_commit(command *master)
{
	name = "queue-commit";
	has_slaves = false;
	this->master = master;
}

int cmd_queue_commit::execute(char *args)
{
	int x = 0;
	for (list<string>::iterator i = queue.begin(); i != queue.end(); i++) {
		cout << " >>> Doing step " << ++x << " of " << queue.size() << "..." << endl;
		// execute(*i);
	}
}

int cmd_queue_commit::validate(char *args)
{
	return true;
}

void cmd_queue_commit::refresh_completion()
{
}




cmd_queue_commit_say::cmd_queue_commit_say(command *master)
{
	name = "queue-commit-say";
	this->master = master;
	has_slaves = false;
}

int cmd_queue_commit_say::execute(char *args)
{
	int x = 0;
	command::set_answer(string(args));
	for (list<string>::iterator i = queue.begin(); i != queue.end(); i++) {
		cout << " >>< Doing step " << ++x << " of " << queue.size() << "..." << endl;
		// execute(*i);
	}
	command::set_answer("");
}

int cmd_queue_commit_say::validate(char *args)
{
	return 0;
}

void cmd_queue_commit_say::refresh_completion()
{
}
