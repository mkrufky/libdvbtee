GPP=$(CROSS_COMPILE)g++
CXX       = $(GPP)
CC        = $(CXX)
STRIP     = $(CROSS_COMPILE)strip

INSTALL_DIR = ../usr/bin

DEBUG = -g -ggdb
INCLUDES = -I../usr/lib -I../libdvbtee -I../libdvbtee_server -I../usr/include
CPPFLAGS = -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread -fPIC $(DEBUG) -D_GNU_SOURCE -D__USE_LARGEFILE64
LDADD = -L../libdvbtee_server -L../libdvbtee -L../usr/lib -ldvbtee_server -ldvbtee -ldvbpsi
LIBS = -L../libdvbtee_server -L../libdvbtee -L../usr/lib -ldvbtee_server -ldvbtee -ldvbpsi

shared: libdvbtee libdvbtee_server
	$(GPP) $(INCLUDES) $(CPPFLAGS) server.cpp -o server $(LIBS)

static: libdvbtee libdvbtee_server
	$(GPP) $(INCLUDES) $(CPPFLAGS) server.cpp ../libdvbtee/libdvbtee.a ../libdvbtee_server/libdvbtee_server.a -o server

libdvbtee:
	make -C ../libdvbtee

libdvbtee_server:
	make -C ../libdvbtee_server

.PHONY: all

clean::
	$(RM) -f server *~

debug: clean
	DEBUG="-DDEBUG -DVERBOSE=1"  make

install:
	if test ! -d $(INSTALL_DIR) ; then mkdir -p  $(INSTALL_DIR) ; fi
	@cp -u -v server $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/server

all: shared
