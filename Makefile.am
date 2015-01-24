SUBDIRS = \
  freakz \
  os

AM_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/freakz @CONTIKI_CFLAGS@

# XXX: if APPLE
AM_LDFLAGS = -Wl,-undefined,error -Wl,-all_load

lib_LTLIBRARIES = libfreakz.la

libfreakz_la_SOURCES = \
  test/native/test_sim.c

libfreakz_la_LIBADD = \
  freakz/libfreakzfreakz.a \
  os/libfreakzos.a

dist_doc_DATA = AUTHORS ChangeLog COPYING NEWS README
