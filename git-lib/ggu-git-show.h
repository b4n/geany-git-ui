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

#ifndef H_GGU_GIT_SHOW
#define H_GGU_GIT_SHOW

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"

G_BEGIN_DECLS


#define GGU_TYPE_GIT_SHOW             (ggu_git_show_get_type())
#define GGU_GIT_SHOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_GIT_SHOW, GguGitShow))
#define GGU_GIT_SHOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_GIT_SHOW, GguGitShowClass))
#define GGU_IS_GIT_SHOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_GIT_SHOW))
#define GGU_IS_GIT_SHOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_GIT_SHOW))
#define GGU_GIT_SHOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_GIT_SHOW, GguGitShowClass))


typedef struct _GguGitShow        GguGitShow;
typedef struct _GguGitShowClass   GguGitShowClass;
typedef struct _GguGitShowPrivate GguGitShowPrivate;

struct _GguGitShow
{
  GguGit parent_instance;
  GguGitShowPrivate *priv;
};

struct _GguGitShowClass
{
  GguGitClass parent_class;
};


GType             ggu_git_show_get_type             (void) G_GNUC_CONST;
GguGitShow       *ggu_git_show_new                  (void);
void              ggu_git_show_show_async           (GguGitShow          *self,
                                                     const gchar         *dir,
                                                     const gchar         *rev,
                                                     const gchar         *file,
                                                     gboolean             diff,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);
const gchar      *ggu_git_show_show_finish          (GguGitShow          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);
void              ggu_git_list_files_changed_async  (GguGitShow          *self,
                                                     const gchar         *dir,
                                                     const gchar         *rev,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);
GList            *ggu_git_list_files_changed_finish (GguGitShow          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);
void              ggu_git_blame_async               (GguGitShow          *self,
                                                     const gchar         *dir,
                                                     const gchar         *rev,
                                                     const gchar         *file,
                                                     GCancellable        *cancellable,
                                                     GAsyncReadyCallback  callback,
                                                     gpointer             user_data);
GList            *ggu_git_blame_finish              (GguGitShow          *self,
                                                     GAsyncResult        *result,
                                                     GError             **error);


G_END_DECLS

#endif /* guard */
