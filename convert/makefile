#
# PURPOSE: makefile for COBE BCT common router
# $Header: //bctquad3/HOME/BCT_Development/Common/convert/rcs/makefile 1.2 2001/08/16 22:48:59 ms10234 Exp ms10234 $
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

CPPSRCS=enum_hit_collect.cpp new_drvr_collect.cpp new_drvr_element.cpp \
old_cfg_collect.cpp old_cfg_element.cpp

CPPOBJS=${CPPSRCS:.cpp=.o}

SRCS=${CPPSRCS}
OBJS= ${CPPOBJS}

all: $(LIBDIR)/$(CFGCONVERTLIB) 

$(LIBDIR)/$(CFGCONVERTLIB) : $(OBJS) 
        $(RM) $@
        wlib -b -c -n -q $@ +$(OBJS)

#
# setup commonsharedtargets.mk
#

lib: $(LIBDIR)/$(CFGCONVERTLIB)

# clean
MISC= $(LIBDIR)/$(CFGCONVERTLIB)

# install
TODIR=convert

# include clean,cleanexe,install & depend targets
include $(COMMONSHAREDTARGETS)

