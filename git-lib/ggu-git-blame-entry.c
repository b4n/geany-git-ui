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

#include "ggu-git-blame-entry.h"

#include <glib.h>
#include <glib-object.h>

#include "ggu-glib-compat.h"


G_DEFINE_BOXED_TYPE (GguGitBlameEntry,
                     ggu_git_blame_entry,
                     ggu_git_blame_entry_ref,
                     ggu_git_blame_entry_unref)


/**
 * ggu_git_blame_entry_new:
 * 
 * Creates a new #GguGitBlameEntry
 * 
 * Returns: A new #GguGitBlameEntry
 */
GguGitBlameEntry *
ggu_git_blame_entry_new (void)
{
  GguGitBlameEntry *entry;
  
  entry = g_slice_alloc0 (sizeof *entry);
  entry->ref_count = 1;
  
  return entry;
}

/**
 * ggu_git_blame_entry_ref:
 * @entry: A #GguGitBlameEntry
 * 
 * Adds a reference to a #GguGitBlameEntry.
 * 
 * Returns: @entry
 */
GguGitBlameEntry *
ggu_git_blame_entry_ref (GguGitBlameEntry *entry)
{
  g_atomic_int_inc (&entry->ref_count);
  return entry;
}

/**
 * ggu_git_blame_entry_unref:
 * @entry: A #GguGitBlameEntry
 * 
 * Drops a reference from a #GguGitBlameEntry.  If the entry's reference count
 * drops to 0, the entree is destroyed.
 */
void
ggu_git_blame_entry_unref (GguGitBlameEntry *entry)
{
  if (g_atomic_int_dec_and_test (&entry->ref_count)) {
    g_free (entry->hash);
    g_free (entry->author);
    g_slice_free1 (sizeof *entry, entry);
  }
}
