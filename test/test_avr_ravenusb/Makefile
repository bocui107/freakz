TEST_RAVEN_USB = 1
TARGET = avr-ravenusb

all: test_avr_ravenusb

CFLAGS += -DTEST_RAVENUSB
SOURCEDIRS += .
CONTIKI_SOURCEFILES += test_zdo.c test_zcl.c test_data.c test_app.c
CONTIKIDIRS += . $(FREAKUSB_PATH)
