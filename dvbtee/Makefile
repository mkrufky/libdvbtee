GPP=$(CROSS_COMPILE)g++
CXX       = $(GPP)
CC        = $(CXX)
STRIP     = $(CROSS_COMPILE)strip

INSTALL_DIR = ../usr/bin

DEBUG = -g -ggdb
INCLUDES = -I../usr/lib -I../libdvbtee -I../libdvbtee_server -I../usr/include
CPPFLAGS = -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread -fPIC $(DEBUG)
LDADD = -L../libdvbtee_server -L../libdvbtee -L../usr/lib -ldvbtee_server -ldvbtee -ldvbpsi
LIBS = -L../libdvbtee_server -L../libdvbtee -L../usr/lib -ldvbtee_server -ldvbtee -ldvbpsi

shared: libdvbtee
	$(GPP) $(INCLUDES) $(CPPFLAGS) dvbtee.cpp -o dvbtee $(LIBS)

static: libdvbtee
	$(GPP) $(INCLUDES) $(CPPFLAGS) dvbtee.cpp ../libdvbtee/libdvbtee.a ../libdvbtee_server/libdvbtee_server.a -o dvbtee

libdvbtee:
	make -C ../libdvbtee
	make -C ../libdvbtee_server

.PHONY: all

clean::
	$(RM) -f dvbtee *~

debug: clean
	DEBUG="-DDEBUG -DVERBOSE=1"  make

install:
	if test ! -d $(INSTALL_DIR) ; then mkdir -p  $(INSTALL_DIR) ; fi
	@cp -u -v dvbtee $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/dvbtee

all: shared
