#
# PURPOSE: makefile for COBE BCT common router
# $Header: K:/BCT_Development/Common/router/rcs/makefile 1.12 2001/05/24 23:16:12 jl11312 Exp jl11312 $
# NB:Do NOT save & try to run this file with DOS CR/LFs
# Set MKS option Configuration->Personal->Checkout->UNIX
# symptom:cant open ../commonrules.mk
#

# define include files
COMMONRULES=../commonrules.mk
COMMONSHAREDTARGETS=$(LOCAL_TOP)/commonsharedtargets.mk

# pull in common project make environment
include $(COMMONRULES)

# set non-portable stack size option
STACKSIZE=32
include $(LOCAL_TOP)/stacksize.mk
# set non-portable offset option
OFFSET=40
include $(LOCAL_TOP)/offset.mk

GATEWAY=$(BINDIR)/gateway
PEARLYGATE=$(BINDIR)/pearlygate
PROGRAMS=$(GATEWAY) $(PEARLYGATE)

MQUEUE_LIB=/usr/lib/mqueue3r.lib

CPPSRCS=\
        new.cpp dispatch.cpp phdspat.cpp spoofdsp.cpp \
        debugclass.cpp callback.cpp kernaltime.cpp timedispatch.cpp
CPPOBJS=${CPPSRCS:.cpp=.o}

CSRCS=gateway.c router.c pearlyg.c sinver.c \
        crc.c mq_check.c error.c debug.c version.c hal_notify.c
COBJS=${CSRCS:.c=.o}

SRCS=${CPPSRCS} ${CSRCS}
OBJS= ${CPPOBJS} ${COBJS}

all: $(LIBDIR)/$(ROUTERLIB) $(PROGRAMS)

$(GATEWAY): gateway.o version.o $(LIBDIR)/$(ROUTERLIB)
        $(LD) $(LDFLAGS) gateway.o version.o $(ROUTERLIB) -o $(GATEWAY)

$(PEARLYGATE) : pearlyg.o version.o $(LIBDIR)/$(ROUTERLIB)
        $(LD) $(LDFLAGS) pearlyg.o version.o $(ROUTERLIB) -o $(PEARLYGATE)

$(LIBDIR)/$(ROUTERLIB) : new.o dispatch.o crc.o sinver.o phdspat.o kernaltime.o timedispatch.o \
                spoofdsp.o mq_check.o error.o debugclass.o debug.o callback.o router.o hal_notify.o
                $(LIB_BLD_CMD) $(LIBDIR)/$(ROUTERLIB) +$(MQUEUE_LIB) +new.o +dispatch.o +crc.o \
                +sinver.o +phdspat.o +spoofdsp.o +mq_check.o +error.o +router.o \
                +kernaltime.o +timedispatch.o +debug.o +debugclass.o +callback.o +hal_notify.o

#
# setup commonsharedtargets.mk
#

lib: $(LIBDIR)/$(ROUTERLIB)
# clean
MISC= $(LIBDIR)/$(ROUTERLIB)

# install
TODIR=router
INSTALLED=$(PROGRAMS)

# include clean,cleanexe,install & depend targets
include $(COMMONSHAREDTARGETS)

