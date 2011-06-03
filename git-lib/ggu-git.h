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

#ifndef H_GGU_GIT
#define H_GGU_GIT

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS


#define GGU_TYPE_GIT            (ggu_git_get_type ())
#define GGU_GIT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_GIT, GguGit))
#define GGU_GIT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_GIT, GguGitClass))
#define GGU_IS_GIT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_GIT))
#define GGU_IS_GIT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_GIT))
#define GGU_GIT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_GIT, GguGitClass))

#define GGU_GIT_ERROR           (ggu_git_error_quark ())

enum
{
  GGU_GIT_ERROR_CRASHED,
  GGU_GIT_ERROR_INCOMPLETE_RESULT,
  GGU_GIT_ERROR_INVALID_RESULT,
  GGU_GIT_ERROR_FAILED
};


typedef struct _GguGit        GguGit;
typedef struct _GguGitClass   GguGitClass;
typedef struct _GguGitPrivate GguGitPrivate;

typedef void    (*GguGitParseOutputFunc)    (GguGit              *self,
                                             const gchar         *output,
                                             GSimpleAsyncResult  *result,
                                             GCancellable        *cancellable);

struct _GguGit
{
  GObject parent_instance;
  GguGitPrivate *priv;
};

struct _GguGitClass
{
  GObjectClass parent_class;
};

GType             ggu_git_get_type              (void) G_GNUC_CONST;
GQuark            ggu_git_error_quark           (void) G_GNUC_CONST;
void              _ggu_git_run_async            (GguGit                *self,
                                                 gchar                **argv,
                                                 GguGitParseOutputFunc  parse_output,
                                                 gint                   priority,
                                                 GCancellable          *cancellable,
                                                 GAsyncReadyCallback    callback,
                                                 gpointer               user_data);
gpointer          _ggu_git_run_finish           (GguGit        *self,
                                                 GAsyncResult  *result,
                                                 GError       **error);
const gchar      *ggu_git_get_dir               (GguGit *self);
void              ggu_git_set_dir               (GguGit      *self,
                                                 const gchar *dir);
const gchar *     ggu_git_get_git_path          (GguGit *self);
void              ggu_git_set_git_path          (GguGit      *self,
                                                 const gchar *path);


G_END_DECLS

#endif /* guard */
