#include <vector>
#include <string>

using namespace std;

#include "command.h"

/* This is the base class for all commands in the 'queue' group.
 * It declares some static variables, shared by the whole group.
 */
class queue_base : public command
{
protected:
	static vector<string*> queue;
};


class cmd_queue : public queue_base
{
public:
	cmd_queue();
	int execute(char *args);
	int validate(char *args);
	void refresh_completion();
};

