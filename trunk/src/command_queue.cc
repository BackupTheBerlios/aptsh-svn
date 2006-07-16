#include <vector>
#include <string>

using namespace std;

#include <cstdio>

#include "command_queue.h"

vector<string*> queue_base::queue = vector<string*>();

cmd_queue::cmd_queue()
{
	name = "queue";
	master = NULL;
	has_slaves = false;
}

int cmd_queue::execute(char *args)
{
	int j = 1;
	for (vector<string*>::iterator i = queue.begin(); i != queue.end(); i++) {
		printf("%d: %s\n", j++, (*i)->c_str());
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

