#
# PURPOSE: builds COBE BCT common crc utilities
# $Header: I:/BCT_Development/Common/softcrc/rcs/makefile 1.5 2000/05/26 21:29:29 MS10234 Exp rm70006 $
# NB:Do NOT save & try to run this file with DOS CR/LFs
# Set MKS option Configuration->Personal->Checkout->UNIX
# symptom:cant open ../commonrules.mk
#

# define include files
COMMONRULES=../commonrules.mk
COMMONSHAREDTARGETS=../commonsharedtargets.mk

# pull in common project make environment
include $(COMMONRULES)

# this is non-portable
STACKSIZE=32
include $(LOCAL_TOP)/stacksize.mk

# built here
CRC_LIB=crc.lib

all : $(PROGRAMS)

# need an extra define
CFLAGS += -DTRACE_CODE=TRACE_DISKCRC

SOFTCRC=$(BINDIR)/softcrc
MACHCRC=$(BINDIR)/machcrc
PROGRAMS = $(SOFTCRC) $(MACHCRC)

OTHSRCS=softcrc.cpp
# checkself goes last s/t it checks everything "above" it
LIBSRCS = crcgen.cpp crcfunctions.cpp checkself.cpp
SRCS=$(LIBSRCS) $(OTHSRCS)
LIBOBJS = $(LIBSRCS:.cpp=.o)
FAKEOBJS = cobecrc.o machcrc.o

all: $(PROGRAMS) $(LIBOBJS) $(LIBDIR)/$(CRC_LIB)

$(LIBDIR)/$(CRC_LIB):$(LIBOBJS)
	$(LIB_BLD_CMD) $(LIBDIR)/$(CRC_LIB) +$(LIBOBJS)

$(SOFTCRC): cobecrc.o $(LIBDIR)/$(CRC_LIB) $(LIBDIR)/routerf.lib
		$(LD) -o $(SOFTCRC)  -g3 -L$(LIBDIR) $(Offsetmacro) $(Stackmacro) cobecrc.o -lcrc.lib -lrouterf.lib

cobecrc.o: softcrc.cpp softcrc.h 
		$(CPP) $(CFLAGS) -DRESTRICT_TO_COBE softcrc.cpp
		mv softcrc.o cobecrc.o

$(MACHCRC): machcrc.o crcgen.o $(LIBDIR)/routerf.lib
		$(LD) -o $(MACHCRC) $(LDFLAGS) machcrc.o crcgen.o \
		-l crc.lib -l routerf.lib

machcrc.o: softcrc.cpp softcrc.h
		$(CPP) $(CFLAGS) -DRESTRICT_TO_MACHINE softcrc.cpp
		mv softcrc.o machcrc.o

crcgen.o: crcgen.cpp 
		$(CPP) $(CFLAGS) crcgen.cpp

checkself.o: checkself.cpp checkself.h
		$(CPP) $(CFLAGS) checkself.cpp

crcfunctions.o: crcfunctions.cpp
		$(CPP) $(CFLAGS) ./crcfunctions.cpp

OBJS=$(LIBOBJS) $(FAKEOBJS)
MISC=$(LIBDIR)/$(CRC_LIB)

TODIR=softcrc
INSTALLED=$(PROGRAMS)
include $(COMMONSHAREDTARGETS)

