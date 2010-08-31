
#include <glib.h>

#ifndef H_GIT_UTILS
#define H_GIT_UTILS

G_BEGIN_DECLS


void      git_list_free_full    (GList      *l,
                                 GFreeFunc   f);


G_END_DECLS

#endif /* guard */
