#
# PURPOSE: makefile for COBE BCT common router
# $Header: Z:/BCT_Development/Common/ROUTER/rcs/MAKEFILE 1.6 1999/08/31 17:53:06 BS04481 Exp MS10234 $
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
ROUTER=$(BINDIR)/router
TCP_GATE=$(BINDIR)/tcp_gate
PROGRAMS=$(GATEWAY) $(PEARLYGATE) $(ROUTER) $(TCP_GATE)

MQUEUE_LIB=/usr/lib/mqueue3r.lib

CPPSRCS=tcp_gate.cpp \
	msg.cpp new.cpp dispatch.cpp sinver.cpp phdspat.cpp spoofdsp.cpp \
	DebugClass.cpp RegisterMsgs.cpp
CPPOBJS=${CPPSRCS:.cpp=.o}

CSRCS=gateway.c router.c pearlyg.c \
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

$(ROUTER): router.o $(LIBDIR)/$(ROUTERLIB)
	$(LD) $(LDFLAGS) router.o $(ROUTERLIB) -o $(ROUTER)

$(LIBDIR)/$(ROUTERLIB) : msg.o new.o dispatch.o crc.o sinver.o phdspat.o \
		spoofdsp.o mq_check.o error.o DebugClass.o debug.o RegisterMsgs.o
		$(LIB_BLD_CMD) $(LIBDIR)/$(ROUTERLIB) +$(MQUEUE_LIB) +msg.o +new.o +dispatch.o +crc.o \
		+sinver.o +phdspat.o +spoofdsp.o +mq_check.o +error.o \
		+debug.o +DebugClass.o RegisterMsgs.o

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

# NOTE: Do not edit below this line
## dependencies will be put after this line... ##
tcp_gate.o:../include/PreRegisterMsgs.h
tcp_gate.o:../include/RegisterMsgs.hpp
tcp_gate.o:../include/an2msgs.h
tcp_gate.o:../include/buffmsg.hpp
tcp_gate.o:../include/buffmsgs.h
tcp_gate.o:../include/crc.h
tcp_gate.o:../include/dispatch.hpp
tcp_gate.o:../include/error.h
tcp_gate.o:../include/intmsgs.h
tcp_gate.o:../include/msg.hpp
tcp_gate.o:../include/msghdr.h
tcp_gate.o:../include/reply.hpp
tcp_gate.o:../include/sinver.h
tcp_gate.o:tcp_gate.cpp
msg.o:../include/crc.h
msg.o:../include/dispatch.hpp
msg.o:../include/error.h
msg.o:../include/intmsgs.h
msg.o:../include/msg.hpp
msg.o:../include/msghdr.h
msg.o:msg.cpp
new.o:../include/error.h
new.o:new.cpp
dispatch.o:../include/buffmsg.hpp
dispatch.o:../include/buffmsgs.h
dispatch.o:../include/crc.h
dispatch.o:../include/dispatch.hpp
dispatch.o:../include/error.h
dispatch.o:../include/intmsgs.h
dispatch.o:../include/msg.hpp
dispatch.o:../include/msghdr.h
dispatch.o:../include/sinver.h
dispatch.o:dispatch.cpp
dispatch.o:mq_check.h
phdspat.o:../include/crc.h
phdspat.o:../include/dispatch.hpp
phdspat.o:../include/intmsgs.h
phdspat.o:../include/msg.hpp
phdspat.o:../include/msghdr.h
phdspat.o:../include/phdspat.hpp
phdspat.o:../include/sinver.h
phdspat.o:phdspat.cpp
spoofdsp.o:../include/buffmsg.hpp
spoofdsp.o:../include/buffmsgs.h
spoofdsp.o:../include/crc.h
spoofdsp.o:../include/dispatch.hpp
spoofdsp.o:../include/error.h
spoofdsp.o:../include/intmsgs.h
spoofdsp.o:../include/msg.hpp
spoofdsp.o:../include/msghdr.h
spoofdsp.o:../include/sinver.h
spoofdsp.o:../include/spoofdsp.hpp
spoofdsp.o:spoofdsp.cpp
DebugClass.o:../include/error.h
DebugClass.o:Debug.hpp
DebugClass.o:DebugClass.cpp
RegisterMsgs.o:../include/RegisterMsgs.hpp
RegisterMsgs.o:../include/an2msgs.h
RegisterMsgs.o:../include/buffmsg.hpp
RegisterMsgs.o:../include/buffmsgs.h
RegisterMsgs.o:../include/crc.h
RegisterMsgs.o:../include/dispatch.hpp
RegisterMsgs.o:../include/error.h
RegisterMsgs.o:../include/intmsgs.h
RegisterMsgs.o:../include/msg.hpp
RegisterMsgs.o:../include/msghdr.h
RegisterMsgs.o:../include/reply.hpp
RegisterMsgs.o:../include/sinver.h
RegisterMsgs.o:RegisterMsgs.cpp
gateway.o:../include/crc.h
gateway.o:../include/error.h
gateway.o:../include/msghdr.h
gateway.o:../include/sinver.h
gateway.o:gateway.c
router.o:../include/crc.h
router.o:../include/error.h
router.o:../include/msghdr.h
router.o:../include/sinver.h
router.o:mq_check.h
router.o:router.c
pearlyg.o:../include/error.h
pearlyg.o:../include/sinver.h
pearlyg.o:pearlyg.c
crc.o:../include/crc.h
crc.o:crc.c
mq_check.o:../include/error.h
mq_check.o:mq_check.c
mq_check.o:mq_check.h
error.o:../include/error.h
error.o:error.c
debug.o:../include/debug.h
debug.o:../include/error.h
debug.o:debug.c
