
CPPFLAGS = -I/usr/include -Wall



bin_PROGRAMS = aptsh

# For some reason, libexecdir=${libexecdir}/aptsh doesn't work.
#led=$(libexecdir)
#libexecdir=$(led)/aptsh
libexec_PROGRAMS = aptsh_ls aptsh_rls

man_MANS = aptsh.1 aptsh_howto.1
EXTRA_DIST = $(man_MANS)

aptsh_ls_SOURCES = ls.c
aptsh_ls_CFLAGS = -Wall @CFLAGS@

aptsh_rls_SOURCES = rls.c
aptsh_rls_CFLAGS = -Wall @CFLAGS@

aptsh_SOURCES = \
	apt_cmds.cc\
	apt_cmds.h\
	column.cc\
	column.h\
	dpkg_complete.cc\
	dpkg_complete.h\
	read_index.cc\
	read_index.h\
	string_utils.cc\
	string_utils.h\
	config_parse.c\
	config_parse.h\
	main.cc

aptsh_LDFLAGS = \
	-avoid-version

aptsh_LDADD = \
	-lreadline -lapt-pkg

aptsh_CPPFLAGS = \
	-Wall @CPPFLAGS@

