CONTIKI = ../os
ZIGBEE_PATH = ../freakz
ROOT_DIR = ..
FREAKUSB_PATH = ../freakusb

SOURCEDIRS += .

PROJECT_DIR = ./test_avr_ravenusb
PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . $(FREAKUSB_PATH) $(RAVEN_USB_DIR)
SOURCEDIRS += .
CONTIKI_SOURCEFILES += test_app.c test_zdo.c test_zcl.c test_data.c

include $(PROJECT_DIR)/Makefile
include $(CONTIKI)/Makefile.include
