
#include <glib.h>

#ifndef H_GGU_GIT_UTILS
#define H_GGU_GIT_UTILS

G_BEGIN_DECLS


void      ggu_git_list_free_full    (GList      *l,
                                     GFreeFunc   f);


G_END_DECLS

#endif /* guard */
