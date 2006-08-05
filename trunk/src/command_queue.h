
#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

/* This is the base class for all commands in the 'queue' group.
 * It declares some static variables, shared by the whole group.
 */
class queue_base : public command
{
public:
	/* Add new command to the queue. */
	static void add(string &cmd_text);
protected:
	static list<string> queue;
};


class cmd_queue : public queue_base
{
public:
	cmd_queue();
	int execute(char *args);
	int validate(char *args);
};


class cmd_queue_remove : public queue_base
{
public:
	cmd_queue_remove(command *master);
	int execute(char *args);
	int validate(char *args);
};

class cmd_queue_clear : public queue_base
{
public:
	cmd_queue_clear(command *master);
	int execute(char *args);
	int validate(char *args);
};

class cmd_queue_commit : public queue_base
{
public:
	cmd_queue_commit(command *master);
	int execute(char *args);
	int validate(char *args);
};

class cmd_queue_commit_say : public queue_base
{
public:
	cmd_queue_commit_say(command *master);
	int execute(char *args);
	int validate(char *args);
};

#endif

