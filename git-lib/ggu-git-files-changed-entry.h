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

#ifndef H_GGU_GIT_FILES_CHANGED_ENTRY
#define H_GGU_GIT_FILES_CHANGED_ENTRY

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GGU_TYPE_GIT_FILES_CHANGED_ENTRY (ggu_git_files_changed_entry_get_type ())


typedef struct _GguGitFilesChangedEntry GguGitFilesChangedEntry;
struct _GguGitFilesChangedEntry
{
  gint    ref_count;
  
  gchar  *hash; /* the revision for which these changes applies */
  gchar  *path;
  guint   added;
  guint   removed;
};


GType                     ggu_git_files_changed_entry_get_type  (void) G_GNUC_CONST;
GguGitFilesChangedEntry  *ggu_git_files_changed_entry_new       (void);
GguGitFilesChangedEntry  *ggu_git_files_changed_entry_ref       (GguGitFilesChangedEntry *entry);
void                      ggu_git_files_changed_entry_unref     (GguGitFilesChangedEntry *entry);


G_END_DECLS

#endif /* guard */
