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

#include "ggu-git-log-entry.h"

#include <glib.h>
#include <glib-object.h>

#include "ggu-glib-compat.h"


G_DEFINE_BOXED_TYPE (GguGitLogEntry,
                     ggu_git_log_entry,
                     ggu_git_log_entry_ref,
                     ggu_git_log_entry_unref)


GguGitLogEntry *
ggu_git_log_entry_new (void)
{
  GguGitLogEntry *entry;
  
  entry = g_slice_alloc0 (sizeof *entry);
  entry->ref_count = 1;
  
  return entry;
}

GguGitLogEntry *
ggu_git_log_entry_ref (GguGitLogEntry *entry)
{
  g_atomic_int_inc (&entry->ref_count);
  return entry;
}

void
ggu_git_log_entry_unref (GguGitLogEntry *entry)
{
  if (g_atomic_int_dec_and_test (&entry->ref_count)) {
    g_free (entry->hash);
    g_free (entry->date);
    g_free (entry->author);
    g_free (entry->summary);
    g_free (entry->details);
    g_slice_free1 (sizeof *entry, entry);
  }
}
