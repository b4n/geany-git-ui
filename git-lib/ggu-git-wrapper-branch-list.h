
#include <glib.h>

#ifndef H_GGU_GIT_WRAPPER_BRANCH_LIST
#define H_GGU_GIT_WRAPPER_BRANCH_LIST

G_BEGIN_DECLS


typedef void (*GguGitBranchListCallback)  (GList/*<gchar *>*/  *branches,
                                           const gchar         *current_branch,
                                           const GError        *error,
                                           gpointer             data);

void            ggu_git_branch_list     (const gchar           *dir,
                                         GguGitBranchListCallback  callback,
                                         gpointer               data);


G_END_DECLS

#endif /* guard */
