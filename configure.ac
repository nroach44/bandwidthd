AC_INIT(bandwidthd, 2.0.1-autotools)
# Process this file with autoconf to produce a configure script.
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([config.h])

### Needed for $X stuff
# Seems to be needed for certain systems.
LIBS="-lpthread $LIBS"

# May be required for libgd on bsd
AC_PATH_X
if test -n "$x_libraries" && test "x$x_libraries" != xNONE ; then
	LDFLAGS="$LDFLAGS -L$x_libraries"
fi

# Required for solaris
AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(resolv, inet_aton)

# Required for openbsd png library
AC_CHECK_LIB(m, pow)

# Required for gd under netbsd
AC_CHECK_LIB(iconv, libiconv_open)

#Headers missing on cygwin
AC_CHECK_HEADERS([arpa/nameser.h])
AC_CHECK_HEADERS([resolv.h],,,
	[#include <netinet/in.h>
	#include <arpa/nameser.h>])

### Required stuff
# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LEX

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions. This block is a relic from the original config script,
# They seem to be completely unneeded checks but the ones that work on a system that
# builds the poject fine are enabled, the AC_FUNC checks need files that were not
# included with the source.
#AC_FUNC_MALLOC
AC_HEADER_STDC
#AC_FUNC_MKTIME
AC_TYPE_SIGNAL
#AC_FUNC_STAT

TCPHDR_SPORT="undefined"
AC_CHECK_MEMBER(struct tcphdr.source,TCPHDR_SPORT="source",,[#include <netinet/tcp.h>])
AC_CHECK_MEMBER(struct tcphdr.th_sport,TCPHDR_SPORT="th_sport",,[#include <netinet/tcp.h>])
if test "$TCPHDR_SPORT" = "undefined" ; then
	AC_MSG_ERROR([Could not determine source port field name for tcphdr structure])
fi
AC_DEFINE_UNQUOTED(TCPHDR_SPORT,$TCPHDR_SPORT,[source port field name in tcphdr])

TCPHDR_DPORT="undefined"
AC_CHECK_MEMBER(struct tcphdr.dest,TCPHDR_DPORT="dest",,[#include <netinet/tcp.h>])
AC_CHECK_MEMBER(struct tcphdr.th_dport,TCPHDR_DPORT="th_dport",,[#include <netinet/tcp.h>])
if test "$TCPHDR_DPORT" = "undefined" ; then
	AC_MSG_ERROR([Could not determine dest port field name for tcphdr structure])
fi
AC_DEFINE_UNQUOTED(TCPHDR_DPORT,$TCPHDR_DPORT,[dest port field name in tcphdr])

# Testing for libpng without having libzlib will cause the test to fail on some systems
AC_CHECK_LIB(z, zlibVersion, , AC_MSG_ERROR([Bandwidthd needs libzlib to be installed]))
AC_CHECK_LIB(png, png_read_info, ,[AC_MSG_ERROR([Bandwidthd needs libpng to be installed])])
AC_CHECK_LIB(gd, gdImageCreate, ,[AC_MSG_ERROR([Bandwidthd needs libgd to be installed])])

#libpcap needs libusb-1.0 if it is installed
AC_CHECK_LIB(usb-1.0, libusb_init, ,[AC_MSG_WARN([libusb-1.0 not found, building or configuration errors may result])])
AC_CHECK_LIB(pcap, pcap_open_live, ,
	[AC_CHECK_LIB(wpcap, pcap_open_live, ,[AC_MSG_ERROR([Bandwidthd needs libpcap to be installed])])])

AC_CHECK_FUNCS([alarm gethostbyaddr inet_ntoa memset strdup strftime pcap_findalldevs])

# Checking for optional stuff
PKG_CHECK_MODULES([LIBPQ], libpq, has_postgres=true, has_postgres=false)
PKG_CHECK_MODULES([LIBSQLITE3], sqlite3, has_sqlite=true, has_sqlite=false)

# Checks for header files.
AC_HEADER_DIRENT

AC_CHECK_HEADERS([gd.h],,
	[AC_CHECK_HEADERS([gd/gd.h],,
		AC_MSG_ERROR([Bandwidthd cannot find gd.h]))])

AC_CHECK_HEADERS(gdfonts.h,,
	[AC_CHECK_HEADERS(gd/gdfonts.h,,
		AC_MSG_ERROR([Bandwidthd cannot find gdfonts.h]))])

AC_CHECK_HEADERS([pcap.h],,
	[AC_MSG_ERROR([Bandwidthd cannot find pcap.h])])

AC_CHECK_HEADERS([arpa/inet.h errno.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h sys/wait.h syslog.h unistd.h],,
	[AC_MSG_ERROR([Bandwidthd cannot find some header files])])

# Set directory names

# We won't ask for confdir because by default autoconf covers it
config_file="${sysconfdir}/bandwidthd.conf"

AC_ARG_WITH([htdocs-dir], AS_HELP_STRING([--with-htdocs-dir=DIR], [Sets default htdocs directory.  Default is LOCALSTATEDIR/bandwidthd/htdocs]),
	htdocs_dir=${with_htdocs_dir},
	htdocs_dir="${localstatedir}/bandwidthd/htdocs")

AC_ARG_WITH([log-dir], AS_HELP_STRING([--with-log-dir=DIR], [Sets default cdf log directory.  Default is LOCALSTATEDIR/bandwidthd]),
	log_dir=${with_log_dir},
	log_dir="${localstatedir}/bandwidthd")

AC_ARG_WITH([extension-dir], AS_HELP_STRING([--with-extension-dir=DIR], [Sets default extension directory.  Default is LOCALSTATEDIR/bandwidthd/extensions]),
	extension_dir=${with_extension_dir},
	extension_dir="${localstatedir}/bandwidthd/extensions")

AC_ARG_WITH([postgresql-logging], AS_HELP_STRING([--with-postgresql-logging=false], [(true/false) Disables support for logging to postgresql servers. Defaults to true if the headers are found. Forcing this to true without the appropriate headers will result in compilation errors.]),
	[])

AC_ARG_WITH([sqlite-storage], AS_HELP_STRING([--with-sqlite-storage=false], [(true/false) Disables support for storing data in a sqlite database. Defaults to true if the headers are found. Forcing this to true without the appropriate headers will result in compilation errors.]),
	sqlite=${with_sqlite_storage})

if test "$has_postgres" = "true" ; then
	if test "$with_postgresql_logging" != "false"; 
		then 	CFLAGS="$CFLAGS -D WITH_LIBPQ $LIBPQ_CFLAGS"
			LDFLAGS="$LDFLAGS $LIBPQ_LIBS"
		else AC_MSG_WARN([Building without postgresql target support. (Disabled by argument)])
	fi
else
	AC_MSG_WARN([Building without postgresql target support. (Install libpq-dev to enable this)])
	postgres=false
fi

if test "$has_sqlite" = "true" ; then
	if test "$with_sqlite_storage" != "false"; 
		then 	CFLAGS="$CFLAGS -D WITH_LIBSQLITE3 $LIBSQLITE3_CFLAGS"
			LDFLAGS="$LDFLAGS $LIBSQLITE3_LIBS"
		else AC_MSG_WARN([Building without sqlite log storage support. (Disabled by argument)])
	fi
else
	AC_MSG_WARN([Building without sqlite log storage support. (Install libsqlite3-dev to enable this)])
	sqlite=false
fi

AC_DEFINE_DIR(CONFIG_DIR, ${sysconfdir}, [Location of bandwidthd.conf])
AC_DEFINE_DIR(CONFIG_FILE, config_file, [Name of bandwidthd.conf])
AC_DEFINE_DIR(HTDOCS_DIR, htdocs_dir, [Name of htdocs directory])
AC_DEFINE_DIR(LOG_DIR, log_dir, [Name of log directory])
AC_DEFINE_DIR(EXTENSION_DIR, extension_dir, [Name of the extensions directory])

AM_CONDITIONAL([POSTGRES], [test x$postgres = xtrue])
AC_SUBST([htdocs_dir],[${htdocs_dir}])
AC_SUBST([extension_dir],[${extension_dir}])
AC_SUBST([log_dir],[${log_dir}])
AC_SUBST([sqlite_status],[${sqlite}])
AC_SUBST([postgresql_status],[${postgres}])
AC_OUTPUT(
data/bandwidthd.conf
)
AC_CONFIG_FILES([
	Makefile
	data/Makefile
	data/extensions/Makefile
	data/phphtdocs/Makefile
	data/phphtdocs/manage/Makefile
	src/Makefile
])
AC_OUTPUT

