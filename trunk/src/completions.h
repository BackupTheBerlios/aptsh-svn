
/* Here are some ready to use completing functions. */


/* Returns NULL. */
char *cpl_none(const char *text, int state);

/* All packages completion. */
char *cpl_pkg(const char * text, int state);

/* Installed packages completion. */
char *cpl_pkg_i(const char * text, int state);

/* Main completion.
 * It's used in the main loop, you *probably* won't find
 * it usable anywhere else.
 */
char *cpl_main(const char * text, int state);


