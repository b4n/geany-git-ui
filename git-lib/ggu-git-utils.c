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

#include "ggu-git-utils.h"

#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <glib.h>


/* FIXME: also check whether the path is known of Git, so don't return Git path
 *        for a file Git don't know but that is in the tree? Or we want to have
 *        it so we might add the file... maybe. */
gboolean
ggu_git_parse_path (const gchar *path,
                    gchar      **root_,
                    gchar      **inner_path_)
{
  gboolean  found = FALSE;
  gchar    *root;
  
  root = g_strdup (path);
  while (root && ! found) {
    gchar *guess;
    
    guess = g_build_filename (root, ".git", NULL);
    found = g_file_test (guess, G_FILE_TEST_IS_DIR);
    if (! found) {
      gchar *tmp;
      
      tmp = g_path_get_dirname (root);
      if (g_strcmp0 (tmp, root) == 0) {
        g_free (tmp);
        tmp = NULL;
      }
      g_free (root);
      root = tmp;
    }
    g_free (guess);
  }
  if (found) {
    if (root_) {
      *root_ = g_strconcat (root, G_DIR_SEPARATOR_S, NULL);
    }
    if (inner_path_) {
      gsize len = strlen (root);
      
      while (path[len] == G_DIR_SEPARATOR || path[len] == '/') {
        len++;
      }
      *inner_path_ = g_strdup (&path[len]);
    }
  }
  g_free (root);
  
  return found;
}
