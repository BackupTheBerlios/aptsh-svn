
AC_INIT(aptsh, 0.4, http://aptsh.berlios.de)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/main.cc)

AC_HEADER_STDC

AM_CONFIG_HEADER(src/config.h)

AC_DEFINE(VERSION, "0.0.5")

# Whops... :)
# Well, Aptsh is intended to be used only on Debian-like systems,
# but if you would like to put configuration file somewhere else,
# then change this manually.
sysconfdir=/etc
AC_DEFINE(CONFIG_FILE, "/etc/aptsh.conf", [Aptsh configuration file])

AC_ARG_WITH(builddeb,
  [  --without-builddeb          Don't compile the program for FHS and packaging into deb])

if test "x$with_builddeb" != "xno"; then
	if test "x$prefix" == "xNONE"; then
		SHD="\"$ac_default_prefix/lib/aptsh/\""
	else
		SHD="\"$prefix/lib/aptsh/\""
	fi
else
	if test "x$prefix" == "xNONE"; then
		SHD="\"$ac_default_prefix/libexec/\""
	else
		SHD="\"$prefix/libexec/\""
	fi
fi

AC_DEFINE_UNQUOTED(SHARED_DIR, $SHD, [Aptsh libexec directory])

AC_CHECK_HEADERS([readline/readline.h],, [AC_MSG_ERROR([Can't find libreadline headers!])] )
AC_CHECK_LIB([readline], [readline],, [AC_MSG_ERROR([Readline library not found!])] )

AC_LANG_CPLUSPLUS
AC_CHECK_HEADERS([apt-pkg/cachefile.h],, [AC_MSG_ERROR([Can't find libapt-pkg headers!])] )
AC_CHECK_LIB([apt-pkg], main,, [AC_MSG_ERROR([Readline library not found!])] )

# We'll write ldflags separately for each executable
LIBS=

AC_CHECK_PROGS(APT_GET, apt-get, no)
AC_CHECK_PROGS(APT_CACHE, apt-cache, no)

if test "$APT_GET" == "no"; then
	AC_MSG_WARN([Apt was not found on this system])
elif test "$APT_CACHE" == "no"; then
	AC_MSG_WARN([Apt was not found on this system])
fi

AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_CXX


AC_OUTPUT(Makefile src/Makefile)

