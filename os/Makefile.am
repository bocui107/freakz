AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libfreakzos.a

AM_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/freakz @CONTIKI_CFLAGS@

libfreakzos_a_SOURCES = \
  core/lib/list.c \
  core/lib/mmem.c \
  core/sys/autostart.c \
  core/sys/ctimer.c \
  core/sys/etimer.c \
  core/sys/process.c \
  core/sys/procinit.c \
  core/sys/timer.c \
  platform/native/clock.c \
  platform/native/contiki-main.c
  