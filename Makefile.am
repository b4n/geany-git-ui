
lib_LTLIBRARIES = libgeany-git-ui.la

ggu_compat_sources  = compat/ggu-glib-compat.h

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

ggu_plugin_sources  = src/ggu-message-box.c \
                      src/ggu-message-box.h \
                      src/ggu-message-info.c \
                      src/ggu-message-info.h \
                      src/ggu-panel.c \
                      src/ggu-panel.h \
                      src/ggu-plugin.c \
                      src/ggu-plugin.h \
                      src/ggu-wrap-label.c \
                      src/ggu-wrap-label.h

libgeany_git_ui_la_SOURCES = $(ggu_compat_sources) \
                             $(ggu_git_lib_sources) \
                             $(ggu_plugin_sources)
libgeany_git_ui_la_LIBADD  = $(AM_LIBS)

# test program
#noinst_PROGRAMS = geany-git-ui-test
#
#geany_git_ui_test_SOURCES = geany-git-ui-test.c
#geany_git_ui_test_LDADD   = $(AM_LIBS) libgeany-git-ui.la



ACLOCAL_AMFLAGS = -I build/m4
AM_CFLAGS       = $(GLIB_CFLAGS) -Igit-lib/ -Icompat/
AM_LIBS         = $(GLIB_LIBS)
