
struct fc_option
{
	string name_short, name_long;
	compl_funct_type completion;
	fc_option(string name_short, string name_long, compl_funct_type completion);
};


template<class child_name>
class full_completion : public command
{
public:
	void refresh_completion();

	/* Completing functions for libreadline */
	static char *cpl_long_opts(const char *text, int state);
	static char *cpl_short_opts(const char *text, int state);
protected:
	static list<fc_option> options;
};


class cmd_dpkg : public full_completion<cmd_dpkg>
{
public:
	cmd_dpkg();
	int execute(char *args);
};

