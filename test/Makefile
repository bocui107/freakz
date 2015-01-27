CONTIKI = ../os
ZIGBEE_PATH = ../freakz
ROOT_DIR = ..
FREAKUSB_PATH = ../freakusb

SOURCEDIRS += .
CURDIR := .

ifneq ($(BUILD_DIR),)
saved-output := $(BUILD_DIR)

# Attempt to create a output directory.
$(shell [ -d ${BUILD_DIR} ] || mkdir -p ${BUILD_DIR})

# Verify if it was successful.
BUILD_DIR := $(shell cd $(BUILD_DIR) && /bin/pwd)
$(if $(BUILD_DIR),,$(error output directory "$(saved-output)" does not exist))
endif

OBJTREE		:= $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR))
SRCTREE		:= $(CURDIR)
TOPDIR		:= $(SRCTREE)
export  TOPDIR SRCTREE OBJTREE

MKCONFIG	:= $(SRCTREE)/mkconfig
export MKCONFIG


ifeq ($(CURDIR)/config.mk,$(wildcard $(CURDIR)/config.mk))
# load other configuration
include $(SRCTREE)/config.mk

PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . $(FREAKUSB_PATH) $(RAVEN_USB_DIR)
SOURCEDIRS += .

include $(SRCTREE)/$(BOARD_NAME)/Makefile
include $(CONTIKI)/Makefile.include

else
all:
	$(error "System not configured - see README")
endif

unconfig:
	@rm -f $(obj)/config.mk

%_config::
	@$(MKCONFIG) -A $(@:_config=)


test_sim_config: unconfig
	@$(MKCONFIG) $(@:_config=) test_sim 

test_avr_raven_config: unconfig
	@$(MKCONFIG) $(@:_config=) test_avr_raven

test_avr_ravenusb_config: unconfig
	@$(MKCONFIG) $(@:_config=) test_avr_ravenusb
