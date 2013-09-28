CONTIKI = ../../os
ZIGBEE_PATH = ../../freakz
ROOT_DIR = ../..
TEST_RAVEN = 1

all: test_avr_raven

CFLAGS += -DTEST_RAVEN
SOURCEDIRS += .
CONTIKI_SOURCEFILES +=
PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += .

include $(CONTIKI)/Makefile.include
