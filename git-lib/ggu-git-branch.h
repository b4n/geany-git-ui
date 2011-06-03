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

#ifndef H_GGU_GIT_BRANCH
#define H_GGU_GIT_BRANCH

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"

G_BEGIN_DECLS


#define GGU_TYPE_GIT_BRANCH             (ggu_git_branch_get_type ())
#define GGU_GIT_BRANCH(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_GIT_BRANCH, GguGitBranch))
#define GGU_GIT_BRANCH_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_GIT_BRANCH, GguGitBranchClass))
#define GGU_IS_GIT_BRANCH(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_GIT_BRANCH))
#define GGU_IS_GIT_BRANCH_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_GIT_BRANCH))
#define GGU_GIT_BRANCH_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_GIT_BRANCH, GguGitBranchClass))


typedef struct _GguGitBranch        GguGitBranch;
typedef struct _GguGitBranchClass   GguGitBranchClass;
typedef struct _GguGitBranchPrivate GguGitBranchPrivate;

struct _GguGitBranch
{
  GguGit parent_instance;
  GguGitBranchPrivate *priv;
};

struct _GguGitBranchClass
{
  GguGitClass parent_class;
};


GType             ggu_git_branch_get_type     (void) G_GNUC_CONST;
GguGitBranch     *ggu_git_branch_new          (void);
void              ggu_git_branch_list_async   (GguGitBranch        *self,
                                               const gchar         *dir,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data);
GList            *ggu_git_branch_list_finish  (GguGitBranch        *self,
                                               const gchar        **current,
                                               GAsyncResult        *result,
                                               GError             **error);


G_END_DECLS

#endif /* guard */
