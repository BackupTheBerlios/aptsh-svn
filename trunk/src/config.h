#define CONFIG_FILE "/etc/aptsh.conf"

enum completion {
	INSTALLED,
	AVAILABLE,
	DPKG,
	FS,
	NONE
};

struct commit_item {
	char * text;
	struct commit_item * next;
};

extern struct commit_item * commitlog;
extern struct commit_item * first;

//extern int cl_size;
//extern char ** commitlog;

#define SHARED_FOLDER "/usr/share/aptsh/"

#define VERSION "0.0.3"

