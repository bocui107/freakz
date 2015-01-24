AC_PREREQ(2.59)
AC_INIT(freakz, 0.52)
AC_PROG_CC

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE

LT_INIT

AC_PROG_LIBTOOL

AC_ARG_WITH([contiki],[AS_HELP_STRING([--with-contiki=PATH],[path to Contiki source])])
AS_IF([test x"$with_contiki" != x -a "x$with_contiki" != xno -a "x$with_contiki" != xyes],
  CONTIKI_PATH="$with_contiki"
  CONTIKI_CFLAGS="-I${CONTIKI_PATH}/core/lib -I${CONTIKI_PATH}/core/sys -I${CONTIKI_PATH}/core/ -I${CONTIKI_PATH}/platform/native -I${CONTIKI_PATH}/cpu/native"
  AC_SUBST(CONTIKI_PATH)
  AC_SUBST(CONTIKI_CFLAGS)
)

AC_CONFIG_FILES(
    Makefile
    freakz/Makefile
    include/Makefile
    os/Makefile
)

AC_OUTPUT
