#Possible optimizations -fomit-frame-pointer -ffast-math
OBS= bandwidthd.o graph.o conf.tab.o conf.l.o
LIBS= -L/usr/local/lib -lgd -lpng -lpcap 
CFLAGS= -I/usr/local/include -O3 -Wall
NONWALLCFLAGS= -O3 #-g -DDEBUG

#Uncomment and edit below for postgresql support
CFLAGS += -DPGSQL -I/usr/local/pgsql/include 
LIBS += -L/usr/local/pgsql/lib -lpq

# Debugging stuff
#CFLAGS += -pg -DPROFILE
#CFLAGS += -g 

OS=$(shell uname -s)
ifeq ("$(OS)","Solaris")
	CFLAGS += -DSOLARIS
	LIBS += -lsocket -lnsl -lresolv	-lm
endif

ifeq ("$(OS)","FreeBSD")
	CFLAGS += -DBSD
	LIBS += -liconv
endif

ifeq ("$(OS)", "OpenBSD")
	CFLAGS += -DBSD
	LIBS += -liconv
endif	

all: bandwidthd

bandwidthd: $(OBS) bandwidthd.h
	$(CC) $(CFLAGS) $(OBS) -o bandwidthd $(LIBS) 

conf.tab.c: conf.y
	bison -pbdconfig_ -d conf.y

conf.l.c: conf.l
	lex -Pbdconfig_ -s -i -t -I conf.l > conf.l.c

clean:
	rm -f *.o bandwidthd *~ DEADJOE core

# This clean deletes the flex and bison output files.  You'll need flex and 
# bison to remake them if you use it.
dist-clean:
	rm -f *.o bandwidthd *~ conf.tab.c conf.tab.h conf.l.c DEADJOE

install: all
	if [ ! -d $(DESTDIR)/usr/local/bandwidthd/etc ] ; then mkdir -p $(DESTDIR)/usr/local/bandwidthd/etc ; fi
	if [ ! -d $(DESTDIR)/usr/local/bandwidthd/htdocs ] ; then mkdir -p $(DESTDIR)/usr/local/bandwidthd/htdocs ; fi
	cp bandwidthd $(DESTDIR)/usr/local/bandwidthd	
	if [ ! -f $(DESTDIR)/usr/local/bandwidthd/etc/bandwidthd.conf ] ; then cp etc/bandwidthd.conf $(DESTDIR)/usr/local/bandwidthd/etc/ ; fi
	cp htdocs/legend.gif $(DESTDIR)/usr/local/bandwidthd/htdocs/
	cp htdocs/logo.gif $(DESTDIR)/usr/local/bandwidthd/htdocs/

#**** Stuff where -WALL is turned off to reduce the noise in a compile so I can see my own errors *******************
conf.l.o: conf.l.c
	$(CC) $(NONWALLCFLAGS) -c -o conf.l.o conf.l.c	
