
lib_LTLIBRARIES = libgeany-git-ui.la

ggu_git_lib_sources = git-lib/ggu-git.c \
                      git-lib/ggu-git.h \
                      git-lib/ggu-git-branch.c \
                      git-lib/ggu-git-branch.h \
                      git-lib/ggu-git-log.c \
                      git-lib/ggu-git-log.h \
                      git-lib/ggu-git-log-entry.c \
                      git-lib/ggu-git-log-entry.h \
                      git-lib/ggu-git-show.c \
                      git-lib/ggu-git-show.h \
                      git-lib/ggu-git-utils.c \
                      git-lib/ggu-git-utils.h \
                      git-lib/ggu-git-version.c \
                      git-lib/ggu-git-version.h

libgeany_git_ui_la_SOURCES = $(ggu_git_lib_sources)
libgeany_git_ui_la_LIBADD  = $(AM_LIBS)

# test program
#noinst_PROGRAMS = geany-git-ui-test
#
#geany_git_ui_test_SOURCES = geany-git-ui-test.c
#geany_git_ui_test_LDADD   = $(AM_LIBS) libgeany-git-ui.la



ACLOCAL_AMFLAGS = -I build/m4
AM_CFLAGS       = $(GLIB_CFLAGS) -Igit-lib/
AM_LIBS         = $(GLIB_LIBS)
