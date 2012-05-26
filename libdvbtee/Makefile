GPP=$(CROSS_COMPILE)g++
CXX       = $(GPP)
CC        = $(CXX)
STRIP     = $(CROSS_COMPILE)strip


# Path to sandbox root
SB_ROOT = $(DVBTEE_ROOT)
# lib dir
SB_LIBDIR = $(SB_ROOT)/usr/lib


INSTALL_DIR = $(SB_ROOT)/usr/lib

INCLUDES= -I../usr/include
GPPFLAGS= $(INCLUDES) -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread
CPPFLAGS= $(INCLUDES) -g -Os -Wall -Wno-deprecated -fpermissive -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread
LDADD= -L../usr/lib -ldvbpsi
LIBS= -L../usr/lib -ldvbpsi

objects = parse.o functions.o decode.o channels.o feed.o tune.o atsctext.o

lib_name = libdvbtee

libraries = $(lib_name).so $(lib_name).a

library: $(libraries)

testapp:
	make -C testapp

$(libraries): $(objects)

CPPFLAGS += -fPIC $(DEBUG)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -MMD -o $@ $< $(filter-out %.h %.c,$^)
%: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)
%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD $(LDFLAGS) -o $@ $< $(filter-out %.h %.c,$^) $(LOADLIBES) $(LDLIBS)
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
