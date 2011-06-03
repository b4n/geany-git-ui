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

#include "ggu-git.h"

#include <string.h>
#include <sys/wait.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-glib-compat.h"


GQuark
ggu_git_error_quark (void)
{
  static GQuark q = 0;
  
  if (G_UNLIKELY (q == 0)) {
    q = g_quark_from_static_string ("GguGit");
  }
  
  return q;
}


struct _GguGitPrivate
{
  gchar *git_path;
  gchar *dir;
};

enum
{
  PROP_0,
  
  PROP_GIT_PATH,
  PROP_DIR
};


G_DEFINE_ABSTRACT_TYPE (GguGit,
                        ggu_git,
                        G_TYPE_OBJECT)


static void
ggu_git_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  GguGit *self = GGU_GIT (object);
  
  switch (prop_id) {
    case PROP_GIT_PATH:
      g_value_set_string (value, self->priv->dir);
      break;
    
    case PROP_DIR:
      g_value_set_string (value, self->priv->dir);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  GguGit *self = GGU_GIT (object);
  
  switch (prop_id) {
    case PROP_GIT_PATH:
      ggu_git_set_git_path (self, g_value_get_string (value));
      break;
    
    case PROP_DIR:
      ggu_git_set_dir (self, g_value_get_string (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_finalize (GObject *object)
{
  GguGit *self = GGU_GIT (object);
  
  g_free (self->priv->git_path);
  self->priv->git_path = NULL;
  g_free (self->priv->dir);
  self->priv->dir = NULL;
  
  G_OBJECT_CLASS (ggu_git_parent_class)->finalize (object);
}

static void
ggu_git_class_init (GguGitClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->get_property  = ggu_git_get_property;
  object_class->set_property  = ggu_git_set_property;
  object_class->finalize      = ggu_git_finalize;
  
  g_object_class_install_property (object_class, PROP_GIT_PATH,
                                   g_param_spec_string ("git-path",
                                                        "Git path",
                                                        "Path to Git",
                                                        "git",
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_DIR,
                                   g_param_spec_string ("dir",
                                                        "Directory",
                                                        "Directory in which run Git",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguGitPrivate));
}

static void
ggu_git_init (GguGit *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GGU_TYPE_GIT,
                                            GguGitPrivate);
  
  self->priv->git_path = g_strdup ("git");
  self->priv->dir = NULL;
}


typedef struct _GitOp GitOp;
struct _GitOp
{
  gchar                *git_path;
  gchar                *dir;
  gchar               **argv;
  GguGitParseOutputFunc parse_output;
};

static void
git_op_free (GitOp *op)
{
  g_free (op->git_path);
  g_free (op->dir);
  g_strfreev (op->argv);
  g_free (op);
}

static void
run_thread (GSimpleAsyncResult *result,
            GObject            *object,
            GCancellable       *cancellable)
{
  GguGit       *self  = GGU_GIT (object);
  GguGitClass  *klass = GGU_GIT_GET_CLASS (self);
  GError       *error = NULL;
  gchar        *output_str;
  gchar        *errors_str;
  gint          status;
  GitOp        *op;
  
  if (g_cancellable_set_error_if_cancelled (cancellable, &error)) {
    g_simple_async_result_take_error (result, error);
    return;
  }
  
  op = g_simple_async_result_get_op_res_gpointer (result);
  /* FIXME: would be better not to need the argv to contain the command? */
  g_free (op->argv[0]);
  op->argv[0] = op->git_path;
  op->git_path = NULL;
  
  if (! g_spawn_sync (op->dir, op->argv, NULL, G_SPAWN_SEARCH_PATH,
                      NULL, NULL, &output_str, &errors_str, &status, &error)) {
    g_simple_async_result_take_error (result, error);
  } else if (! WIFEXITED (status)) {
    g_simple_async_result_set_error (result,
                                     GGU_GIT_ERROR, GGU_GIT_ERROR_CRASHED,
                                     "Git crashed");
  } else if (WEXITSTATUS (status) != 0) {
    g_simple_async_result_set_error (result,
                                     GGU_GIT_ERROR, GGU_GIT_ERROR_FAILED,
                                     "Git terminated with error code %d: %s",
                                     WEXITSTATUS (status), errors_str);
  } else {
    if (g_cancellable_set_error_if_cancelled (cancellable, &error)) {
      g_simple_async_result_take_error (result, error);
    } else {
      op->parse_output (self, output_str, result, cancellable);
    }
  }
  
  g_free (output_str);
  g_free (errors_str);
}

/**
 * _ggu_git_run_async:
 * @self: A #GguGit object
 * @argv: A NULL-terminated array of the arguments of the command to spawn
 *       (including the program name)
 * @parse_output: Function to call after the subprocess terminated
 * @priority: The thread priority
 * @cancellable: The user's #GCancellable, or %NULL
 * @callback: The user's #GAsyncReadyCallback
 * @user_data: The user's #GAsyncReadyCallback user data
 * 
 * Runs a subprocess in background and let @parse_output work on its output.
 * All is run in a separate thread, so @parse_output might block without
 * blocking the caller thread.
 * 
 * @parse_output must set the #GSimpleAsyncResult's operation result using
 * g_simple_async_result_set_op_res_gpointer().
 */
void
_ggu_git_run_async (GguGit               *self,
                    gchar               **argv,
                    GguGitParseOutputFunc parse_output,
                    gint                  priority,
                    GCancellable         *cancellable,
                    GAsyncReadyCallback   callback,
                    gpointer              user_data)
{
  GSimpleAsyncResult *result;
  GitOp              *op;
  
  op = g_malloc (sizeof *op);
  op->git_path      = g_strdup (self->priv->git_path);
  op->dir           = g_strdup (self->priv->dir);
  op->argv          = g_strdupv (argv);
  op->parse_output  = parse_output;
  
  result = g_simple_async_result_new (G_OBJECT (self), callback, user_data,
                                      (gpointer) _ggu_git_run_async);
  /* we use result to transfer the operation data to the thread, but it will
   * be overwritten later by the operation. not a problem, we won't need it
   * anymore */
  g_simple_async_result_set_op_res_gpointer (result, op,
                                             (GDestroyNotify) git_op_free);
  g_simple_async_result_run_in_thread (result, run_thread, priority,
                                       cancellable);
  g_object_unref (result);
}

/**
 * _ggu_git_run_finish:
 * @self: A #GguGit object
 * @result: The #GAsyncResult
 * @error: Return loaction for errors, or %NULL to ignore them
 * 
 * Fetches the operation's result, taking care of propagating errors. If an
 * error occurs, it returns %NULL.
 * 
 * If %NULL is a possibly valid result of your operation, you should check
 * whether the error was set if you need to determine whether the operation
 * succeeded or not.
 * 
 * Returns: (transfer none): the operation's result or %NULL on error
 */
gpointer
_ggu_git_run_finish (GguGit        *self,
                     GAsyncResult  *result,
                     GError       **error)
{
  GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);
  
  g_return_val_if_fail (GGU_IS_GIT (self), NULL);
  g_return_val_if_fail (G_IS_SIMPLE_ASYNC_RESULT (result), NULL);
  g_warn_if_fail (g_simple_async_result_get_source_tag (simple) == (gpointer) _ggu_git_run_async);
  
  if (g_simple_async_result_propagate_error (simple, error)) {
    return NULL;
  }
  
  return g_simple_async_result_get_op_res_gpointer (simple);
}

const gchar *
ggu_git_get_dir (GguGit *self)
{
  g_return_val_if_fail (GGU_IS_GIT (self), NULL);
  
  return self->priv->dir;
}

void
ggu_git_set_dir (GguGit      *self,
                 const gchar *dir)
{
  g_return_if_fail (GGU_IS_GIT (self));
  
  if (g_strcmp0 (self->priv->dir, dir) != 0) {
    g_free (self->priv->dir);
    self->priv->dir = g_strdup (dir);
    g_object_notify (G_OBJECT (self), "dir");
  }
}

const gchar *
ggu_git_get_git_path (GguGit *self)
{
  g_return_val_if_fail (GGU_IS_GIT (self), NULL);
  
  return self->priv->git_path;
}

void
ggu_git_set_git_path (GguGit      *self,
                      const gchar *path)
{
  g_return_if_fail (GGU_IS_GIT (self));
  
  if (g_strcmp0 (self->priv->git_path, path) != 0) {
    g_free (self->priv->git_path);
    self->priv->git_path = g_strdup (path);
    g_object_notify (G_OBJECT (self), "git-path");
  }
}
