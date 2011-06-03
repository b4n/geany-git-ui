
AC_PREREQ([2.64])
AC_INIT([Geany Git UI],
        [0.1],
        [ban@herbesfolles.org],
        [geany-git-ui])
AC_CONFIG_SRCDIR([geany-git-ui-test.c])
AC_CONFIG_AUX_DIR([build/aux])
AC_CONFIG_MACRO_DIR([build/m4])
AM_INIT_AUTOMAKE([1.11.1 -Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

LT_PREREQ([2.2.0])
LT_INIT([disable-static])
AC_PROG_CC
AC_PROG_CC_C_O

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.16
                           gio-2.0
                           gtk+-2.0 >= 2.20
                           geany >= 0.21])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
