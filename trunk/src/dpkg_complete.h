
class dpkg_complete
{
	public:
	dpkg_complete(char * word, char * text, int index);
		
	// Actions like -i, -s, -S
	static char * normal(const char * text, int state);
	
	// Long version of above, --install etc.
	static char * normal_double(const char * text, int state);
	
	// Completing only names of files with .deb extension
	static char * fs_deb(const char * text, int state);
		
	// This function returns pointer to the completion function we
	// must use. Actually, this is essential.
	//char* (*check(char * word, char * text, int index))(const char*, int);

	char*(*completion)(const char*, int);
};

