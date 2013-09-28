CONTIKI = ../../os
ZIGBEE_PATH = ../../freakz
ROOT_DIR = ../..
TEST_RAVEN_USB = 1
TARGET = avr-ravenusb

#this is relative to the makefile in /os/platform/avr-ravenusb
FREAKUSB_PATH = ../../freakusb

all: test_avr_ravenusb

CFLAGS += -DTEST_RAVENUSB
SOURCEDIRS += .
CONTIKI_SOURCEFILES += test_zdo.c test_zcl.c test_data.c test_app.c
PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . $(FREAKUSB_PATH)

include $(CONTIKI)/Makefile.include
