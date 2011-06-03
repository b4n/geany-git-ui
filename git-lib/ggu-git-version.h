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

#ifndef H_GGU_GIT_VRESION
#define H_GGU_GIT_VRESION

#include <glib.h>
#include <gio/gio.h>

#include "ggu-git.h"

G_BEGIN_DECLS


void            ggu_git_get_version_async     (GguGit              *self,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data);
gboolean        ggu_git_get_version_finish    (GguGit       *self,
                                               gint          v[4],
                                               GAsyncResult *result,
                                               GError      **error);
void            ggu_git_check_version_async   (GguGit              *self,
                                               gint                 p1,
                                               gint                 p2,
                                               gint                 p3,
                                               gint                 p4,
                                               GCancellable        *cancellable,
                                               GAsyncReadyCallback  callback,
                                               gpointer             user_data);
gboolean        ggu_git_check_version_finish  (GguGit       *self,
                                               GAsyncResult *result,
                                               GError      **error);


G_END_DECLS

#endif /* guard */
