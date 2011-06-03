/*
 * Copyright 2011 Colomban Wendling <ban@herbesfolles.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "ggu-git-branch.h"

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"


struct _GguGitBranchPrivate
{
  
};

enum
{
  PROP_0
};


static void         ggu_git_branch_get_property     (GObject    *object,
                                                     guint       prop_id,
                                                     GValue     *value,
                                                     GParamSpec *pspec);
static void         ggu_git_branch_set_property     (GObject       *object,
                                                     guint          prop_id,
                                                     const GValue  *value,
                                                     GParamSpec    *pspec);
static void         ggu_git_branch_finalize         (GObject *object);


G_DEFINE_TYPE (GguGitBranch,
               ggu_git_branch,
               GGU_TYPE_GIT)


static void
ggu_git_branch_class_init (GguGitBranchClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  GguGitClass  *git_class     = GGU_GIT_CLASS (klass);
  
  /*object_class->get_property  = ggu_git_branch_get_property;
  object_class->set_property  = ggu_git_branch_set_property;
  object_class->finalize      = ggu_git_branch_finalize;*/
  
  /*g_type_class_add_private (klass, sizeof (GguGitBranchPrivate));*/
}

static void
ggu_git_branch_init (GguGitBranch *self)
{
  /*self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GGU_TYPE_GIT_BRANCH,
                                            GguGitBranchPrivate);*/
}

static void
ggu_git_branch_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GguGitBranch *self = GGU_GIT_BRANCH (object);
  
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_branch_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GguGitBranch *self = GGU_GIT_BRANCH (object);
  
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_branch_finalize (GObject *object)
{
  GguGitBranch *self = GGU_GIT_BRANCH (object);
  
  G_OBJECT_CLASS (ggu_git_branch_parent_class)->finalize (object);
}


GguGitBranch *
ggu_git_branch_new (void)
{
  return g_object_new (GGU_TYPE_GIT_BRANCH, NULL);
}


typedef struct _BranchListOp BranchListOp;
struct _BranchListOp
{
  GList        *branches;
  const gchar  *current;
};

static void
branch_list_op_free (BranchListOp *op)
{
  g_list_free_full (op->branches, g_free);
  g_free (op);
}

static void
ggu_git_branch_list_parse_output (GguGit             *obj,
                                  const gchar        *output,
                                  GSimpleAsyncResult *result,
                                  GCancellable       *cancellable)
{
  BranchListOp *op;
  
  op = g_malloc (sizeof *op);
  op->branches = NULL;
  op->current = NULL;
  
  while (*output) {
    gboolean      current = FALSE;
    const gchar  *start;
    gchar        *branch;
    
    if (*output == '*') {
      output++;
      current = TRUE;
    }
    while (*output == ' ') {
      output++;
    }
    for (start = output; *output && *output != '\n'; output++) {
    }
    branch = g_strndup (start, (gsize)(output - start));
    if (*output) {
      output++;
    }
    if (current) {
      op->current = branch;
    }
    op->branches = g_list_prepend (op->branches, branch);
  }
  op->branches = g_list_reverse (op->branches);
  
  g_simple_async_result_set_op_res_gpointer (result, op,
                                             (GDestroyNotify) branch_list_op_free);
}

void
ggu_git_branch_list_async (GguGitBranch        *self,
                           const gchar         *dir,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
  const gchar *argv[] = {
    "git",
    "branch",
    NULL
  };
  
  g_object_set (self, "dir", dir, NULL);
  _ggu_git_run_async (GGU_GIT (self), (gchar **) argv,
                      ggu_git_branch_list_parse_output, G_PRIORITY_DEFAULT,
                      cancellable, callback, user_data);
}

/**
 * ggu_git_branch_list_finish:
 * @self: The #GguGitBranch object
 * @current: Return location for the current branch (this is a pointer to one
 *           of the strings in the returned list)
 * @result: The #GAsyncResult
 * @error: Return location for errors, or %NULL to ignore
 * 
 * 
 * 
 * Returns: (transfer none): A list of branch names, as returned by `git branch`
 */
GList *
ggu_git_branch_list_finish (GguGitBranch *self,
                            const gchar **current,
                            GAsyncResult *result,
                            GError      **error)
{
  BranchListOp *op;
  
  op = _ggu_git_run_finish (GGU_GIT (self), result, error);
  if (! op) {
    return NULL;
  }
  
  if (current) {
    *current = op->current;
  }
  
  return op->branches;
}
