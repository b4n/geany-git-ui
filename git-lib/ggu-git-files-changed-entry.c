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

#include "ggu-git-files-changed-entry.h"

#include <glib.h>
#include <glib-object.h>

#include "ggu-glib-compat.h"


G_DEFINE_BOXED_TYPE (GguGitFilesChangedEntry,
                     ggu_git_files_changed_entry,
                     ggu_git_files_changed_entry_ref,
                     ggu_git_files_changed_entry_unref)


GguGitFilesChangedEntry *
ggu_git_files_changed_entry_new (void)
{
  GguGitFilesChangedEntry *entry;
  
  entry = g_slice_alloc0 (sizeof *entry);
  entry->ref_count = 1;
  
  return entry;
}

GguGitFilesChangedEntry *
ggu_git_files_changed_entry_ref (GguGitFilesChangedEntry *entry)
{
  g_atomic_int_inc (&entry->ref_count);
  return entry;
}

void
ggu_git_files_changed_entry_unref (GguGitFilesChangedEntry *entry)
{
  if (g_atomic_int_dec_and_test (&entry->ref_count)) {
    g_free (entry->path);
    g_slice_free1 (sizeof *entry, entry);
  }
}
