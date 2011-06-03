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

#ifndef H_GGU_GIT_LOG
#define H_GGU_GIT_LOG

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"

G_BEGIN_DECLS


#define GGU_TYPE_GIT_LOG            (ggu_git_log_get_type ())
#define GGU_GIT_LOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_GIT_LOG, GguGitLog))
#define GGU_GIT_LOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_GIT_LOG, GguGitLogClass))
#define GGU_IS_GIT_LOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_GIT_LOG))
#define GGU_IS_GIT_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_GIT_LOG))
#define GGU_GIT_LOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_GIT_LOG, GguGitLogClass))

#define GGU_GIT_LOG_ERROR           (ggu_git_log_error_quark ())

enum
{
  GGU_GIT_LOG_ERROR_INCOMPLETE_RESULT,
  GGU_GIT_LOG_ERROR_INVALID_RESULT,
  GGU_GIT_LOG_ERROR_FAILED
};


typedef struct _GguGitLog        GguGitLog;
typedef struct _GguGitLogClass   GguGitLogClass;
typedef struct _GguGitLogPrivate GguGitLogPrivate;

struct _GguGitLog
{
  GguGit parent_instance;
  GguGitLogPrivate *priv;
};

struct _GguGitLogClass
{
  GguGitClass parent_class;
};


GType             ggu_git_log_get_type        (void) G_GNUC_CONST;
GQuark            ggu_git_log_error_quark     (void) G_GNUC_CONST;
GguGitLog        *ggu_git_log_new             (void);
void              ggu_git_log_log_async       (GguGitLog           *self,
                                               const gchar         *dir,
                                               const gchar         *ref,
                                               const gchar         *file,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data);
GList            *ggu_git_log_log_finish      (GguGitLog           *self,
                                               GAsyncResult        *result,
                                               GError             **error);


G_END_DECLS

#endif /* guard */
