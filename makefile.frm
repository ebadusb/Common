# $Header$
# $Log: makefile.frm $
# Revision 1.3  2007/04/09 20:12:42Z  jl11312
# - moved reserved messages directory (IT 76)
# Revision 1.2  2007/03/28 18:05:00Z  jl11312
# - added check for valid reserved messages directory
# Revision 1.1  2007/03/13 18:05:31Z  jmedusb
# Initial revision
# Revision 1.1  2007/02/07 17:29:51Z  jmedusb
# Initial revision

PRODUCTION_RMSG_DIR := k:\BCT_Development\reserved_messages_perforce_builds
PRODUCTION_RMSG_DIR_ECHO := $(subst \,/, $(PRODUCTION_RMSG_DIR))
LRMBD := latest_rmsg_build_dir.mk
dir_path := $(subst \,/,$(shell cmd /C cd))/

.PHONY: find_rmsg
find_rmsg:
	@if exist .rmsg rmdir /S /Q .rmsg
	@mkdir .rmsg
	@if exist $(LRMBD) (echo Default Reserved Message directory overridden $(dir_path))
	@if exist $(LRMBD) (echo $(subst /,\,$(subst .\,$(dir_path),$(shell cmd /C if exist $(LRMBD) cat $(LRMBD)))) > .rmsg\$(LRMBD))
	@if not exist $(LRMBD) (echo Default Reserved Message directory used)
	@if not exist $(LRMBD) (echo $(subst /,\,$(patsubst .%,$(strip $(PRODUCTION_RMSG_DIR_ECHO))%,$(shell cat $(PRODUCTION_RMSG_DIR_ECHO)\$(LRMBD)))) > .rmsg\$(LRMBD))

.PHONY: verify_rmsg
verify_rmsg:
	@rm -f .rmsg/status
	@if not exist $(RMSGDIR)\create_rmsg.bat echo File create_rmsg.bat not found > .rmsg\status
	@if not exist $(RMSGDIR)\MessageGenerator.exe echo File MessageGenerator.exe not found > .rmsg\status
	@if exist .rmsg\status cat .rmsg\status
	@if not exist .rmsg\status echo OK

