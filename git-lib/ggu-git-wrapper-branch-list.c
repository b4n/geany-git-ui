
#include "ggu-git-wrapper-branch-list.h"

#include <glib.h>
#include <string.h>

#include "ggu-git-utils.h"
#include "ggu-git-wrapper-private.h"



typedef struct _GguGitBranchListPrivate GguGitBranchListPrivate;
struct _GguGitBranchListPrivate
{
  GguGitBranchListCallback  callback;
  gpointer                  callback_data;
};

static GList *
ggu_git_branch_list_parse_output (const gchar  *buf,
                                  const gchar **current_branch)
{
  GList *branches = NULL;
  
  while (*buf) {
    gboolean      current = FALSE;
    const gchar  *start;
    gchar        *branch;
    
    if (*buf == '*') {
      buf++;
      current = TRUE;
    }
    while (*buf == ' ') buf++;
    for (start = buf; *buf && *buf != '\n'; buf++);
    branch = g_strndup (start, (gsize)(buf - start));
    if (*buf) buf++;
    if (current && current_branch) *current_branch = branch;
    branches = g_list_prepend (branches, branch);
  }
  branches = g_list_reverse (branches);
  
  return branches;
}

static void
ggu_git_branch_list_finish_callback (gboolean     success,
                                     gint         return_value,
                                     const gchar *standard_output,
                                     const gchar *standard_error,
                                     gpointer     data)
{
  GguGitBranchListPrivate  *priv = data;
  GList                    *branches = NULL;
  const gchar              *current_branch = NULL;
  GError                   *error = NULL;
  
  if (! success) {
    error = g_error_new_literal (GGU_GIT_WRAPPER_ERROR,
                                 GGU_GIT_WRAPPER_ERROR_CHILD_CRASHED,
                                 "Git crashed");
  } else if (return_value != 0) {
    error = g_error_new (GGU_GIT_WRAPPER_ERROR, GGU_GIT_WRAPPER_ERROR_FAILED,
                         "Git failed: %s", standard_error);
  } else {
    standard_error = NULL;
    branches = ggu_git_branch_list_parse_output (standard_output,
                                                 &current_branch);
  }
  
  priv->callback (branches, current_branch, error, priv->callback_data);
  if (error) g_error_free (error);
  ggu_git_list_free_full (branches, g_free);
  g_slice_free1 (sizeof *priv, priv);
}

void
ggu_git_branch_list (const gchar             *dir,
                     GguGitBranchListCallback callback,
                     gpointer                 data)
{
  GError                   *err = NULL;
  GguGitBranchListPrivate  *priv;
  
  priv = g_slice_alloc (sizeof *priv);
  priv->callback = callback;
  priv->callback_data = data;
  
  if (! ggu_git_wrapper (dir, "branch", NULL,
                         ggu_git_branch_list_finish_callback, priv, &err)) {
    /* FIXME: call this from inside the GMainLoop for the thread things to be
     * the same as a successful call.
     * Use E.g. a timeout (very short) or an idle */
    callback (NULL, NULL, err, data);
    g_error_free (err);
    g_slice_free1 (sizeof *priv, priv);
  }
}
