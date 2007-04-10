# $Header: K:/BCT_Development/vxWorks/Common/rcs/makefile.frm 1.2 2007/03/28 18:05:00Z jl11312 Exp jl11312 $
# $Log: makefile.frm $
# Revision 1.2  2007/03/28 18:05:00Z  jl11312
# - added check for valid reserved messages directory
# Revision 1.1  2007/03/13 18:05:31Z  jmedusb
# Initial revision
# Revision 1.1  2007/02/07 17:29:51Z  jmedusb
# Initial revision

PRODUCTION_RMSG_DIR := \\bctquad3\HOME\BCT_Development\vxWorks\common\reserved_messages_builds
PRODUCTION_RMSG_DIR_ECHO := $(subst \,/, $(PRODUCTION_RMSG_DIR))
LRMBD := latest_rmsg_build_dir.mk

.PHONY: find_rmsg
find_rmsg:
	@rm -f -r .rmsg
	@mkdir .rmsg
	@if exist $(LRMBD) (echo Default Reserved Message directory overridden)
	@if exist $(LRMBD) (echo $(patsubst .%,$(shell cmd /c cd)\.%, $(shell cmd /c if exist $(LRMBD) cat $(LRMBD))) > .rmsg\$(LRMBD))
	@if not exist $(LRMBD) (echo Default Reserved Message directory used)
	@if not exist $(LRMBD) (echo $(patsubst .%,$(strip $(PRODUCTION_RMSG_DIR_ECHO))%, \
							$(shell cat $(PRODUCTION_RMSG_DIR_ECHO)/$(LRMBD))) > .rmsg\$(LRMBD))

.PHONY: verify_rmsg
verify_rmsg:
	@rm -f .rmsg/status
	@if not exist $(RMSGDIR)\create_rmsg.bat echo File create_rmsg.bat not found > .rmsg\status
	@if not exist $(RMSGDIR)\MessageGenerator.exe echo File MessageGenerator.exe not found > .rmsg\status
	@if exist .rmsg\status cat .rmsg\status
	@if not exist .rmsg\status echo OK

