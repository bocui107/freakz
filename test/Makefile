CONTIKI = ../os
ZIGBEE_PATH = ../freakz
ROOT_DIR = ..
FREAKUSB_PATH = ../freakusb

SOURCEDIRS += .

ifneq ($(BUILD_DIR),)
saved-output := $(BUILD_DIR)

# Attempt to create a output directory.
$(shell [ -d ${BUILD_DIR} ] || mkdir -p ${BUILD_DIR})

# Verify if it was successful.
BUILD_DIR := $(shell cd $(BUILD_DIR) && /bin/pwd)
$(if $(BUILD_DIR),,$(error output directory "$(saved-output)" does not exist))
endif # ifneq ($(BUILD_DIR),)

OBJTREE		:= $(if $(BUILD_DIR),$(BUILD_DIR),$(CURDIR))
SRCTREE		:= $(CURDIR)
TOPDIR		:= $(SRCTREE)
export  TOPDIR SRCTREE OBJTREE

MKCONFIG	:= $(SRCTREE)/mkconfig
export MKCONFIG

ifeq ($(wildcard $(SRCTREE)/config.mk),)
$(error "System not configured - see README")
endif

# load other configuration
include $(SRCTREE)/config.mk

PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . $(FREAKUSB_PATH) $(RAVEN_USB_DIR)
SOURCEDIRS += .

include ./native/Makefile
include $(CONTIKI)/Makefile.include

unconfig:
	@rm -f $(obj)/config.mk


%_config::
	@$(MKCONFIG) -A $(@:_config=)

native_config	:	unconfig
	@$(MKCONFIG) $(@:_config=) native
