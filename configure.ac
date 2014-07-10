AC_INIT(bandwidthd, 2.0.1-autotools)
# Process this file with autoconf to produce a configure script.
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LEX


# May be required for libgd on bsd
AC_PATH_X
if test -n "$x_libraries" && test "x$x_libraries" != xNONE ; then
	LDFLAGS="$LDFLAGS -L$x_libraries"
fi

# May be equired for BSD
LDFLAGS="$LDFLAGS -L/usr/local/lib"

CPPFLAGS="$CPPFLAGS -I/usr/local/include"

# Required for solaris
AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(resolv, inet_aton)

# Required for openbsd png library
AC_CHECK_LIB(m, pow)

# Required for gd under netbsd
AC_CHECK_LIB(iconv, libiconv_open)

# Required Libraries
AC_CHECK_LIB(png, png_read_info, ,[AC_MSG_ERROR([Bandwidthd requires but cannot libpng])])
AC_CHECK_LIB(gd, gdImageCreate, ,[AC_MSG_ERROR([Bandwidthd requires but cannot find libgd])])
AC_CHECK_LIB(pcap, pcap_open_live, ,
	[AC_CHECK_LIB(wpcap, pcap_open_live, ,[AC_MSG_ERROR([Bandwidthd requires but cannot find libpcap])])])

AC_CHECK_LIB(sqlite3, sqlite3_open, ,AC_MSG_WARN([sqlite3 is recommended for storage of data but cannot be found]))


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


#Headers missing on cygwin
AC_CHECK_HEADERS([arpa/nameser.h])
AC_CHECK_HEADERS([resolv.h],,,
	[#include <netinet/in.h>
	#include <arpa/nameser.h>])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

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


# Checks for library functions.
#AC_FUNC_MALLOC
#AC_HEADER_STDC
#AC_FUNC_MKTIME
#AC_TYPE_SIGNAL
#AC_FUNC_STAT
#AC_CHECK_FUNCS([alarm gethostbyaddr inet_ntoa memset strdup strftime pcap_findalldevs])


# Set directory names
AC_ARG_WITH([htdocs-dir], AC_HELP_STRING([--with-htdocs-dir=DIR], [Sets default htdocs directory.  Default is LOCALSTATEDIR/bandwidthd/htdocs]),
	htdocs_dir=${with_htdocs_dir},
	htdocs_dir="${localstatedir}/bandwidthd/htdocs")
AC_ARG_WITH([log-dir], AC_HELP_STRING([--with-log-dir=DIR], [Sets default cdf log directory.  Default is LOCALSTATEDIR/bandwidthd]),
	log_dir=${with_log_dir},
	log_dir="${localstatedir}/bandwidthd")
AC_ARG_WITH([extension-dir], AC_HELP_STRING([--with-extension-dir=DIR], [Sets default extension directory.  Default is LOCALSTATEDIR/bandwidthd/extensions]),
	extension_dir=${with_extension_dir},
	extension_dir="${localstatedir}/bandwidthd/extensions")
config_file="${sysconfdir}/bandwidthd.conf"
AC_DEFINE_DIR(CONFIG_FILE, config_file, [Name of bandwidthd.conf])
AC_DEFINE_DIR(LOG_DIR, log_dir, [Name of log directory])
AC_DEFINE_DIR(HTDOCS_DIR, htdocs_dir, [Name of htdocs directory])
AC_DEFINE_DIR(EXTENSION_DIR, extension_dir, [Name of the extensions directory])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
