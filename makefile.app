# @(#) $Header: K:/BCT_Development/vxWorks/Common/rcs/makefile.app 1.2 2002/04/19 12:59:58 jl11312 Exp jl11312 $
#
# Makefile.app - common makefile for vxWorks applications
#
# $Log: makefile.app $
# Revision 1.1  2002/04/18 14:27:47  jl11312
# Initial revision
#

#
# If multiple applications are being built from the same file, then
# the ???_CAT interface must be used (i.e. the list of C, C++, and
# lib files are dumped into the associated .opt/??? files, and
# make is invoked recursively.  This is due to a problem with make,
# which won't work correctly with the automatic dependency generation
# if it has command lines which have variables defined like this:
#
#   APP_CXX_FILES="f1.cpp f2.cpp"
#
# If make needs to rebuild included dependency files, it will
# automatically restart itself, unfortunately with the " characters
# dropped, so that the command line is no longer interpreted correctly.
#
# If only a single app is being built, the makefile can simply define
# APP_C_FILES, APP_CXX_FILES, and APP_LIBS and include this file
# to build the app.
#
# See the Trima project for examples using these two interfaces.
# The file trima/proc/makefile.vx uses the ???_CAT interface to build
# multiple applications.  The file trima/proc/predict/makefile.vx uses
# the direct define interface for a single app.
#
ifdef APP_C_CAT
APP_C_FILES := $(shell if exist .opt\$(APP_C_CAT) cat .opt/$(APP_C_CAT))
endif

ifdef APP_CXX_CAT
APP_CXX_FILES := $(shell if exist .opt\$(APP_CXX_CAT) cat .opt/$(APP_CXX_CAT))
endif

ifdef APP_LIBS_CAT
APP_LIBS := $(shell if exist .opt\$(APP_LIBS_CAT) cat .opt/$(APP_LIBS_CAT))
endif

APP_OBJS := $(addprefix $(CPU)/, $(notdir $(APP_C_FILES:.c=.o))) \
           $(addprefix $(CPU)/, $(notdir $(APP_CXX_FILES:.cpp=.o)))

#
# Ignore APP_MAP for now - mapfile generation causes an
# abnormal termination of ld386 if the -r option is used.
# See SPR 68959 on the windriver web site.
#
#ifdef APP_MAP
#MAP_OPT := -Map $(APP_DIR)/$(APP_MAP)
#else
#MAP_OPT :=
#endif
#
MAP_OPT :=

$(APP_DIR)/$(APP_NAME) : output_dir $(APP_OBJS) $(APP_LIBS)
	@echo Building $(APP_DIR)/$(APP_NAME)
	@if not exist $(APP_DIR) mkdir $(APP_DIR)
	rm -f $@ $(CPU)/.ctdt.c $(CPU)/.ctdt.o $(CPU)/.partialImage.o
	$(LD) $(LDFLAGS) $(APP_OBJS) $(APP_LIBS) -o $(CPU)/.partialImage.o
	$(NM) $(CPU)/.partialImage.o | $(MUNCH) > $(CPU)/.ctdt.c
	$(CC) -fdollars-in-identifiers $(CXXFLAGS) $(CPU)/.ctdt.c -o $(CPU)/.ctdt.o
	$(LD) $(LDFLAGS) $(LD_ARCH_RELOC) $(MAP_OPT) $(CPU)/.partialImage.o $(CPU)/.ctdt.o -o $@

.PHONY: output_dir
output_dir:
	@if not exist $(CPU) mkdir $(CPU)

ifndef FORCE_REBUILD
 ifneq ($(strip $(APP_C_FILES)),)
  -include $(addprefix .dfile/, $(notdir $(APP_C_FILES:.c=.d)))
 endif

 ifneq ($(strip $(APP_CXX_FILES)),)
  -include $(addprefix .dfile/, $(notdir $(APP_CXX_FILES:.cpp=.d)))
 endif
else
 include $(MAINDIR)/makefile.rules
endif

