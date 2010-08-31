
#include "git-wrapper-branch-list.h"

#include <glib.h>
#include <string.h>

#include "git-utils.h"
#include "git-wrapper-private.h"



typedef struct _GitBranchListPrivate GitBranchListPrivate;
struct _GitBranchListPrivate
{
  GitBranchListCallback callback;
  gpointer              callback_data;
};

static GList *
git_branch_list_parse_output (const gchar  *buf,
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
git_branch_list_finish_callback (gboolean     success,
                                 gint         return_value,
                                 const gchar *standard_output,
                                 const gchar *standard_error,
                                 gpointer     data)
{
  GitBranchListPrivate *priv = data;
  GList                *branches = NULL;
  const gchar          *current_branch = NULL;
  GError               *error = NULL;
  
  if (! success) {
    error = g_error_new_literal (GIT_WRAPPER_ERROR,
                                 GIT_WRAPPER_ERROR_CHILD_CRASHED,
                                 "Git crashed");
  } else if (return_value != 0) {
    error = g_error_new (GIT_WRAPPER_ERROR, GIT_WRAPPER_ERROR_FAILED,
                         "Git failed: %s", standard_error);
  } else {
    standard_error = NULL;
    branches = git_branch_list_parse_output (standard_output, &current_branch);
  }
  
  priv->callback (branches, current_branch, error, priv->callback_data);
  if (error) g_error_free (error);
  git_list_free_full (branches, g_free);
  g_slice_free1 (sizeof *priv, priv);
}

void
git_branch_list (const gchar           *dir,
                 GitBranchListCallback  callback,
                 gpointer               data)
{
  GError               *err = NULL;
  GitBranchListPrivate *priv;
  
  priv = g_slice_alloc (sizeof *priv);
  priv->callback = callback;
  priv->callback_data = data;
  
  if (! git_wrapper (dir, "branch", NULL, git_branch_list_finish_callback, priv,
                     &err)) {
    /* FIXME: call this from inside the GMainLoop for the thread things to be
     * the same as a successful call.
     * Use E.g. a timeout (very short) or an idle */
    callback (NULL, NULL, err, data);
    g_error_free (err);
    g_slice_free1 (sizeof *priv, priv);
  }
}
