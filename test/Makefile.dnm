CONTIKI = ../os
ZIGBEE_PATH = ../freakz
ROOT_DIR = ..
FREAKUSB_PATH = ../freakusb

SOURCEDIRS += .

PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . $(FREAKUSB_PATH) $(RAVEN_USB_DIR)
SOURCEDIRS += .

config-targets := 0

ifeq ($(KBUILD_EXTMOD),)
        ifneq ($(filter config %config,$(MAKECMDGOALS)),)
                config-targets := 1
                ifneq ($(filter-out config %config,$(MAKECMDGOALS)),)
                        mixed-targets := 1
                endif
        endif
endif

ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Read arch specific Makefile to set KBUILD_DEFCONFIG as needed.
# KBUILD_DEFCONFIG may point out an alternative default configuration
# used for 'make defconfig'

%_config::
	@$(MKCONFIG) -A $(@:_config=)

else

# load other configuration
include $(srctree)/config.mk

ifeq ($(wildcard ./config.mk),)
$(error "System not configured - see README")
endif
endif

-include $(PROJECT_DIR)/Makefile
include $(CONTIKI)/Makefile.include
