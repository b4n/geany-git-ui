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

#include <glib.h>

#ifndef H_GGU_GIT_UTILS
#define H_GGU_GIT_UTILS

G_BEGIN_DECLS


gboolean  ggu_git_parse_path        (const gchar *path,
                                     gchar      **root_,
                                     gchar      **inner_path_);
gchar    *ggu_git_utf8_ensure_valid (const gchar *str);
gboolean  ggu_git_is_hash           (const gchar *hash);


G_END_DECLS

#endif /* guard */
