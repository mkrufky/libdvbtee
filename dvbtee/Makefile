GPP=$(CROSS_COMPILE)g++
CXX       = $(GPP)
CC        = $(CXX)
STRIP     = $(CROSS_COMPILE)strip


# Path to sandbox root
SB_ROOT = $(DVBTEE_ROOT)
# lib dir
SB_LIBDIR = $(SB_ROOT)/usr/lib


INSTALL_DIR = $(SB_ROOT)/usr/bin

INCLUDES= -I../usr/include
GPPFLAGS= $(INCLUDES) -Wno-deprecated -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread
CPPFLAGS= $(INCLUDES) -g -Os -Wall -Wno-deprecated -fpermissive -Wno-deprecated-declarations -D_FILE_OFFSET_BITS=64 -pthread
LDADD= -L../usr/lib -ldvbpsi
LIBS= -L../usr/lib -ldvbpsi

all:	dvbtee

clean:
	rm -f dvbtee *.o

distclean: clean

DVBTEE= parse.o functions.o decode.o channels.o feed.o tune.o atsctext.o

dvbtee:	$(DVBTEE)
	$(GPP) $(INCLUDES) $(CPPFLAGS) dvbtee.cpp $(DVBTEE) -o $(@) $(LIBS)

install:
	if test ! -d $(INSTALL_DIR) ; then mkdir -p  $(INSTALL_DIR) ; fi
	@cp -u -v dvbtee $(INSTALL_DIR)
	$(STRIP) $(INSTALL_DIR)/dvbtee

.PHONY: all clean distclean install
