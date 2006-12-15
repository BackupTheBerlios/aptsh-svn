
AM_CPPFLAGS = -I/usr/include -Wall



bin_PROGRAMS = aptsh

# For some reason, libexecdir=${libexecdir}/aptsh doesn't work.
#led=$(libexecdir)
#libexecdir=$(led)/aptsh
libexec_PROGRAMS = aptsh_ls aptsh_rls aptsh_printer

man_MANS = aptsh.1
EXTRA_DIST = $(man_MANS)

aptsh_ls_SOURCES = ls.c
aptsh_ls_CFLAGS = -Wall @CFLAGS@

aptsh_rls_SOURCES = rls.c
aptsh_rls_CFLAGS = -Wall @CFLAGS@

aptsh_printer_SOURCES = printer.c
aptsh_printer_CFLAGS = -Wall @CFLAGS@

aptsh_SOURCES = \
	column.cc\
	column.h\
	read_index.cc\
	read_index.h\
	string_utils.cc\
	string_utils.h\
	config_parse.c\
	config_parse.h\
	command.cc\
	command.h\
	command_queue.cc\
	command_queue.h\
	completions.cc\
	completions.h\
	full_completion.cc\
	full_completion.h\
	main.cc\
	main.h

aptsh_LDFLAGS = \
	-avoid-version

aptsh_LDADD = \
	-lreadline -lapt-pkg

aptsh_CPPFLAGS = \
	-Wall @CPPFLAGS@

