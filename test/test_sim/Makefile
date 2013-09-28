CONTIKI = ../../os
ZIGBEE_PATH = ../../freakz
ROOT_DIR = ../..

all: test_sim

CFLAGS += -DTEST_SIM
SOURCEDIRS += .
CONTIKI_SOURCEFILES += test_app.c test_zdo.c test_zcl.c test_data.c
PROJECT_OBJECTFILES += ${addprefix $(OBJECTDIR)/,${call oname, $(CONTIKI_SOURCEFILES)}}
CONTIKIDIRS += . ../test_sim ../../freakz/driver/sim

include $(CONTIKI)/Makefile.include
