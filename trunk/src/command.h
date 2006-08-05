
#ifndef COMMAND_H
#define COMMAND_H

/* The interface for all commands */
class command
{
public:
	string name;
	/* Extra names for command. */
	vector<string> aliases;
	/* Please note, that you must override this function - it's 
	 * pure virtual (well, we don't need commands that do nothing).
	 */
	virtual int execute(char *args) = 0;
	/* Update pointer referring to a completing function for librealine.
	 * May use libreadline's interface directly for deciding,
	 * as well as some ready-to-use completing functions (see below).
	 */
	virtual void refresh_completion();
	/* Pointer referrring to a completing function for libreadline,
	 * that should be used for this command. If the completion may vary,
	 * please override refresh_completion() with proper code (it is executed
	 * before every usage of 'completion').
	 * Actually, in C++ a function can't return a pointer to another function,
	 * that's why things are that crazy.
	 */
	char* (*completion)(const char*, int);
	/* If it's a root command, then master == NULL. */
	command *master;
	/* Does this command have any slave-functions? */
	bool has_slaves;

	/* Return 1 if any errors, 0 if clear.
	 * If don't want to provide validations, please don't
	 * put any code in this.
	 */
	virtual int validate(char *args);

	/* Text that's displayed when run
	 * help command_name
	 */
	string help_text;

	/* Set string that will (repeately) feed command's
	 * input. If it's clear (""), then commands
	 * will be just system()-ized.
	 */
	static void set_answer(string answer);

	command();
	virtual ~command();

protected:
	static string command_answer;
};

//extern vector<command*> commands;

class command_vector : public vector<command*>
{
public:
	command* locate_by_name(string name);
};

/* Here we keep all commands. */
extern command_vector commands;

/* Some basic apt-get/apt-cache commands. */
class cmd_aptize : public command
{
public:
	typedef enum validations {
		ALL,
		INSTALLED,
		NONE
	};

	cmd_aptize(string name, string sh_command, char* (*completion)(const char*, int), validations validation, string help_text = "");
	cmd_aptize(string name, string sh_command, vector<string> &aliases, char* (*completion)(const char*, int), validations validation, string help_text = "");
	 
	int execute(char *args);

	int validate(char *args);

	~cmd_aptize();
private:
	string sh_command;
	validations validation;
};


/* Ready class to be used by commands that just launch external program by system().
 * No completion, no validation.
 */
class cmd_systemize : public command
{
public:
	cmd_systemize(string name, string sh_cmd, bool ignore_args = false, command *master = NULL, bool has_slaves = false);
	cmd_systemize(string name, string sh_cmd, string help_text);
	cmd_systemize(string name, string sh_cmd, char* (*completion)(const char*, int), bool ignore_args = false, command *master = NULL, bool has_slaves = false);
	int execute(char *args);
private:
	string sh_cmd;
	bool ignore_args;
};


class cmd_whatis : public command
{
public:
	cmd_whatis();
	int execute(char *args);
};


class cmd_orphans : public command
{
public:
	cmd_orphans();
	int execute(char *args);
};


class cmd_orphans_all : public command
{
public:
	/* Give pointer to 'orphans' command. */
	cmd_orphans_all(command *master);
	int execute(char *args);
};

class cmd_ls : public command
{
public:
	cmd_ls();
	int execute(char *args);
};


class cmd_rls : public command
{
public:
	cmd_rls();
	int execute(char *args);
};

class cmd_dpkg : public command
{
public:
	cmd_dpkg();
	int execute(char *args);
	void refresh_completion();
};

class cmd_dump_cfg : public command
{
public:
	cmd_dump_cfg();
	int execute(char *args);
};

class cmd_help : public command
{
public:
	cmd_help();
	int execute(char *args);
};

class cmd_quit : public command
{
public:
	cmd_quit();
	int execute(char *args);
};


#endif

