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

#include "ggu-git-version.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"


typedef struct _GetVersionOp GetVersionOp;
struct _GetVersionOp
{
  gboolean  success;
  gint      v[4];
};

static void
ggu_git_get_version_parse_output (GguGit             *obj,
                                  const gchar        *output,
                                  GSimpleAsyncResult *result,
                                  GCancellable       *cancellable)
{
  GRegex       *re;
  GMatchInfo   *infos;
  GError       *err = NULL;
  GetVersionOp *op;
  
  op = g_malloc (sizeof *op);
  op->success = FALSE;
  
  re = g_regex_new ("([0-9]+)(?:\\.([0-9]+)(?:\\.([0-9]+)(?:\\.([0-9]+))?)?)?",
                    0, 0, &err);
  if (! re) {
    g_warning ("Regex compiltaion failed: %s", err->message);
    g_error_free (err);
  } else if (g_regex_match (re, output, 0, &infos)) {
    op->success = TRUE;
    op->v[0] = atoi (g_match_info_fetch (infos, 1));
    op->v[1] = atoi (g_match_info_fetch (infos, 2));
    op->v[2] = atoi (g_match_info_fetch (infos, 3));
    op->v[3] = atoi (g_match_info_fetch (infos, 4));
  }
  g_simple_async_result_set_op_res_gpointer (result, op, g_free);
}

void
ggu_git_get_version_async (GguGit              *self,
                           GCancellable        *cancellable,
                           GAsyncReadyCallback  callback,
                           gpointer             user_data)
{
  const gchar *argv[] = {
    "git",
    "--version",
    NULL
  };
  
  _ggu_git_run_async (GGU_GIT (self), (gchar **) argv,
                      ggu_git_get_version_parse_output, G_PRIORITY_DEFAULT,
                      cancellable, callback, user_data);
}

gboolean
ggu_git_get_version_finish (GguGit       *self,
                            gint          v[4],
                            GAsyncResult *result,
                            GError      **error)
{
  GetVersionOp *op;
  
  op = _ggu_git_run_finish (GGU_GIT (self), result, error);
  if (! op) {
    return FALSE;
  }
  
  if (v) {
    v[0] = op->v[0];
    v[1] = op->v[1];
    v[2] = op->v[2];
    v[3] = op->v[3];
  }
  
  return op->success;
}


/* check version */

typedef struct _CheckVersionOp CheckVersionOp;
struct _CheckVersionOp
{
  gboolean            valid;
  gint                v[4];
  
  GError             *error;
  GAsyncReadyCallback callback;
  gpointer            user_data;
};

static void
ggu_git_check_version_get_version_callback (GObject      *obj,
                                            GAsyncResult *result,
                                            gpointer      user_data)
{
  gint            v[4] = { 0 };
  CheckVersionOp *op = user_data;
  
  if (! ggu_git_get_version_finish (GGU_GIT (obj), v, result, &op->error)) {
    op->valid = FALSE;
  } else if (v[0] > op->v[0] ||
             (v[0] == op->v[0] &&
              (v[1] > op->v[1] ||
               (v[1] == op->v[1] &&
                (v[2] > op->v[2] ||
                 (v[2] == op->v[2] &&
                  (v[3] >= op->v[3]))))))) {
    op->valid = TRUE;
  } else {
    op->valid = FALSE;
    op->error = g_error_new (G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
                             "Git is not recent enough: requested version "
                             "%d.%d.%d.%d but %d.%d.%d.%d present.",
                             op->v[0], op->v[1], op->v[2], op->v[3],
                             v[0], v[1], v[2], v[3]);
  }
  /* haxx: we call the callback manually, since we wrap it */
  g_simple_async_result_set_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result),
                                             op, NULL);
  op->callback (obj, result, op->user_data);
  
  if (op->error) {
    g_error_free (op->error);
  }
  g_free (op);
}

void
ggu_git_check_version_async (GguGit              *self,
                             gint                 p1,
                             gint                 p2,
                             gint                 p3,
                             gint                 p4,
                             GCancellable        *cancellable,
                             GAsyncReadyCallback  callback,
                             gpointer             user_data)
{
  CheckVersionOp *op;
  
  op = g_malloc (sizeof *op);
  op->valid = FALSE;
  op->v[0] = p1;
  op->v[1] = p2;
  op->v[2] = p3;
  op->v[3] = p4;
  op->error = NULL;
  op->callback = callback;
  op->user_data = user_data;
  
  ggu_git_get_version_async (self, cancellable,
                             ggu_git_check_version_get_version_callback, op);
}

gboolean
ggu_git_check_version_finish (GguGit       *self,
                              GAsyncResult *result,
                              GError      **error)
{
  CheckVersionOp *op;
  
  op = g_simple_async_result_get_op_res_gpointer (G_SIMPLE_ASYNC_RESULT (result));
  if (error) {
    *error = op->error;
    op->error = NULL;
  }
  return op->valid;
}
