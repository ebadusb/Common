#
# PURPOSE: makefile for COBE BCT common router
# $Header: Y:/BCT_Development/Common/ROUTER/rcs/MAKEFILE 1.9 2000/05/26 21:28:55 MS10234 Exp ms10234 $
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
TCP_GATE=$(BINDIR)/tcp_gate
PROGRAMS=$(GATEWAY) $(PEARLYGATE) $(TCP_GATE)

MQUEUE_LIB=/usr/lib/mqueue3r.lib

CPPSRCS=tcp_gate.cpp \
	msg.cpp new.cpp dispatch.cpp phdspat.cpp spoofdsp.cpp \
	debugclass.cpp registermsgs.cpp callback.cpp
CPPOBJS=${CPPSRCS:.cpp=.o}

CSRCS=gateway.c router.c pearlyg.c sinver.c \
	crc.c mq_check.c error.c debug.c
COBJS=${CSRCS:.c=.o}

SRCS=${CPPSRCS} ${CSRCS}
OBJS= ${CPPOBJS} ${COBJS}

all: $(LIBDIR)/$(ROUTERLIB) $(PROGRAMS)

$(TCP_GATE): tcp_gate.o $(LIBDIR)/$(ROUTERLIB)
	$(LD) $(LDFLAGS) tcp_gate.o $(ROUTERLIB) -l socket3r -o $@

$(GATEWAY): gateway.o $(LIBDIR)/$(ROUTERLIB)
	$(LD) $(LDFLAGS) gateway.o $(ROUTERLIB) -o $(GATEWAY)

$(PEARLYGATE) : pearlyg.o $(LIBDIR)/$(ROUTERLIB)
	$(LD) $(LDFLAGS) pearlyg.o $(ROUTERLIB) -o $(PEARLYGATE)

$(LIBDIR)/$(ROUTERLIB) : msg.o new.o dispatch.o crc.o sinver.o phdspat.o \
		spoofdsp.o mq_check.o error.o debugclass.o debug.o registermsgs.o callback.o router.o
		$(LIB_BLD_CMD) $(LIBDIR)/$(ROUTERLIB) +$(MQUEUE_LIB) +msg.o +new.o +dispatch.o +crc.o \
		+sinver.o +phdspat.o +spoofdsp.o +mq_check.o +error.o +router.o \
		+debug.o +debugclass.o +registermsgs.o +callback.o

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

