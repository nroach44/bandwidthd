BUILT_SOURCES = parser.h
AM_YFLAGS = -d
bin_PROGRAMS = bandwidthd
bandwidthd_SOURCES = bandwidthd.c graph.c extensions.c sqlight.c parser.y lexer.l bandwidthd.h pgsql.c

bandwidthd_LDADD = $(LIBOBJS)
