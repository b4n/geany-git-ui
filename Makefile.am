
lib_LTLIBRARIES = libgeany-git-ui.la

libgeany_git_ui_la_SOURCES = git-lib/ggu-git-wrapper-private.c \
                             git-lib/ggu-git-wrapper-log.c \
                             git-lib/ggu-git-wrapper-branch-list.c \
                             git-lib/ggu-git-utils.c
libgeany_git_ui_la_LIBADD  = $(AM_LIBS)

# test program
noinst_PROGRAMS = geany-git-ui-test

geany_git_ui_test_SOURCES = geany-git-ui-test.c
geany_git_ui_test_LDADD   = $(AM_LIBS) libgeany-git-ui.la



ACLOCAL_AMFLAGS = -I build/m4
AM_CFLAGS       = $(GLIB_CFLAGS) -Igit-lib/
AM_LIBS         = $(GLIB_LIBS)
