#define CONFIG_FILE "/etc/aptsh.conf"

enum completion {
	INSTALLED,
	AVAILABLE,
	FS,
	NONE
};

struct commit_item {
	char * text;
	commit_item * next;
};

extern struct commit_item * commitlog;
extern struct commit_item * first;

//extern int cl_size;
//extern char ** commitlog;

#define SHARED_FOLDER "/usr/share/aptsh/"

