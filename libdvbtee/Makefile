GPP=$(CROSS_COMPILE)g++
CXX       = $(GPP)
CC        = $(CXX)
STRIP     = $(CROSS_COMPILE)strip


INSTALL_DIR = $(DVBTEE_ROOT)/usr/lib

DEBUG = -g -ggdb
INCLUDES= -I../usr/include
GPPFLAGS= $(INCLUDES) -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread -D_GNU_SOURCE -D__USE_LARGEFILE64
CPPFLAGS= $(INCLUDES) -g -Os -Wall -Wno-deprecated -fpermissive -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread $(DEBUG)  -D_GNU_SOURCE -D__USE_LARGEFILE64
LDADD= -L../usr/lib -ldvbpsi
LIBS= -L../usr/lib -ldvbpsi

objects = parse.o functions.o decode.o channels.o feed.o tune.o atsctext.o \
	  output.o desc.o rbuf.o listen.o stats.o demux.o

lib_name = libdvbtee

libraries = $(lib_name).so $(lib_name).a

library: $(libraries)

include $(wildcard *.d)

$(libraries): $(objects)

CPPFLAGS += -fPIC $(DEBUG)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -MMD -o $@ $< $(filter-out %.h %.c,$^)
%.o: %.cpp
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -MMD -o $@ $< $(filter-out %.h %.cpp,$^)
%: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)
%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD $(LDFLAGS) -o $@ $< $(filter-out %.h %.c,$^) $(LOADLIBES) $(LDLIBS)
%: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD $(LDFLAGS) -o $@ $< $(filter-out %.h %.cpp,$^) $(LOADLIBES) $(LDLIBS)
%.so:
	$(CC) -shared -o $@ $^
%.a:
	$(AR) rcs $@ $^
clean::
	$(RM) -f $(prerequisites) $(objects) $(libraries) $(binaries) $(removing) *~ *.d

debug: clean
	DEBUG="-DDEBUG -DVERBOSE=1"  make

install:
	cp libdvbtee.so $(INSTALL_DIR)

all: library
