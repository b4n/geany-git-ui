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

#ifndef H_GGU_GIT_BLAME_ENTRY
#define H_GGU_GIT_BLAME_ENTRY

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GGU_TYPE_GIT_BLAME_ENTRY (ggu_git_blame_entry_get_type ())


typedef struct _GguGitBlameEntry GguGitBlameEntry;
/**
 * GguGitBlameEntry:
 * @line: The line this entry is for
 * @hash: The hash of the commit lats modifying the line
 * @author: The author of the last commit modifying this line
 * 
 * Blame information for a line.
 */
struct _GguGitBlameEntry
{
  /*< private >*/
  gint      ref_count;
  
  /*< public >*/
  /*guint64   hash;*/
  gulong    line;
  gchar    *hash;
  gchar    *author;
};


GType             ggu_git_blame_entry_get_type  (void) G_GNUC_CONST;
GguGitBlameEntry *ggu_git_blame_entry_new       (void);
GguGitBlameEntry *ggu_git_blame_entry_ref       (GguGitBlameEntry *entry);
void              ggu_git_blame_entry_unref     (GguGitBlameEntry *entry);


G_END_DECLS

#endif /* guard */
