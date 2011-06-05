
lib_LTLIBRARIES = libgeany-git-ui.la

ggu_compat_sources  = compat/ggu-glib-compat.h \
                      compat/ggu-gtk-compat.h

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

ggu_generated_plugin_sources  = src/ggu-marshal.c \
                                src/ggu-marshal.h
ggu_plugin_sources  = src/ggu-auto-link-label.c \
                      src/ggu-auto-link-label.h \
                      src/ggu-fade-out-widget.c \
                      src/ggu-fade-out-widget.h \
                      src/ggu-history-store.c \
                      src/ggu-history-store.h \
                      src/ggu-history-view.c \
                      src/ggu-history-view.h \
                      src/ggu-message-box.c \
                      src/ggu-message-box.h \
                      src/ggu-message-info.c \
                      src/ggu-message-info.h \
                      src/ggu-panel.c \
                      src/ggu-panel.h \
                      src/ggu-plugin.c \
                      src/ggu-plugin.h \
                      src/ggu-wrap-label.c \
                      src/ggu-wrap-label.h


EXTRA_DIST = src/ggu-marshal.list
CLEANFILES = $(ggu_generated_plugin_sources)

libgeany_git_ui_la_SOURCES = $(ggu_compat_sources) \
                             $(ggu_git_lib_sources) \
                             $(ggu_generated_plugin_sources) \
                             $(ggu_plugin_sources)
libgeany_git_ui_la_LIBADD  = $(AM_LIBS)

# test program
#noinst_PROGRAMS = geany-git-ui-test
#
#geany_git_ui_test_SOURCES = geany-git-ui-test.c
#geany_git_ui_test_LDADD   = $(AM_LIBS) libgeany-git-ui.la



ACLOCAL_AMFLAGS = -I build/m4
AM_CFLAGS       = $(GLIB_CFLAGS) -I$(top_srcdir)/git-lib/ -I$(top_srcdir)/compat/
AM_LIBS         = $(GLIB_LIBS)


GLIB_GENMARSHAL_FLAGS = --prefix=_ggu_cclosure_marshal

%.c: $(top_srcdir)/%.list %.h Makefile
	$(AM_V_GEN) \
	(echo '#include "$(shell basename $@ .c).h"' && \
	 $(GLIB_GENMARSHAL) $(GLIB_GENMARSHAL_FLAGS) --body $<) > $@
%.h: $(top_srcdir)/%.list Makefile
	$(AM_V_GEN) \
	$(GLIB_GENMARSHAL) $(GLIB_GENMARSHAL_FLAGS) --header $< > $@
