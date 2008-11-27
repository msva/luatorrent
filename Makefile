CC= g++
CFLAGS= -g -O2 -Wall -shared -fpic -I /usr/include/lua5.1/
AR= ar rcu
RANLIB= ranlib
RM= rm -f
LIBS=-ltorrent-rasterbar -lboost_filesystem -lpthread
OUTLIB=luatorrent.so

LDFLAGS= $(LIBS)

OBJS = main.o torrent_handle.o torrent_info.o torrent_session.o

all: luatorrent

clean:
	$(RM) $(OBJS) $(OUTLIB)

luatorrent: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTLIB) $(LDFLAGS)

echo:
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "LDFLAGS = $(LDFLAGS)"

main.o: main.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_handle.o: torrent_handle.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_info.o: torrent_info.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)
torrent_session.o: torrent_session.cpp utils.h
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all 
