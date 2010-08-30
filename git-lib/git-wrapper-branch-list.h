
#include <glib.h>

#ifndef H_GIT_WRAPPER_BRANCH_LIST
#define H_GIT_WRAPPER_BRANCH_LIST

G_BEGIN_DECLS


typedef void (*GitBranchListCallback) (GList/*<gchar *>*/  *branches,
                                       const gchar         *current_branch,
                                       const gchar         *error,
                                       gpointer             data);

void            git_branch_list     (const gchar           *dir,
                                     GitBranchListCallback  callback,
                                     gpointer               data);


G_END_DECLS

#endif /* guard */
