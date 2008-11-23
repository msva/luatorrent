PLAT= none

CC= g++
CFLAGS= -g -O2 -Wall $(MYCFLAGS)
AR= ar rcu
RANLIB= ranlib
RM= rm -f
LIBS= $(MYLIBS) -ltorrent-rasterbar -lboost_filesystem -lpthread
OUTLIB=luatorrent.so

LDFLAGS= $(LIBS)

MYCFLAGS=
MYLDFLAGS=
MYLIBS=

OBJS = main.o torrent_handle.o torrent_info.o torrent_session.o

PLATS=linux macosx

build: luatorrent

clean:
	$(RM) $(OBJS) $(OUTLIB)

luatorrent: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTLIB) $(LDFLAGS)

none:
	@echo "Please choose a platform:"
	@echo "   $(PLATS)"

all: 
	$(MAKE) none

echo:
	@echo "PLAT = $(PLAT)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "MYCFLAGS = $(MYCFLAGS)"
	@echo "MYLDFLAGS = $(MYLDFLAGS)"
	@echo "MYLIBS = $(MYLIBS)"


linux:
	$(MAKE) build MYCFLAGS="-shared -fpic -I /usr/include/lua5.1/" MYLIBS="-L /usr/local/lib"

macosx:
	$(MAKE) build MACOSX_DEPLOYMENT_TARGET="10.3" MYCFLAGS="-fno-common -undefined dynamic_lookup -bundle -I /usr/local/include/libtorrent -I /usr/local/include -I /usr/local/include/boost-1_34_1" MYLIBS=""

main.o: main.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_handle.o: torrent_handle.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_info.o: torrent_info.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_session.o: torrent_session.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)


.PHONY: all $(PLATS) clean none 
