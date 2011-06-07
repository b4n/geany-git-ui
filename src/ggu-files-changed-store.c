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

/* TODO: would be better to implement this with a simpler data structure than
 *       GtkTreeModel (e.g. GPtrArray or whatever) since our data is known.
 *       But this would need to implement the whole GtkTreeModel iface... */

#include "ggu-files-changed-store.h"

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-git-files-changed-entry.h"


G_DEFINE_TYPE (GguFilesChangedStore,
               ggu_files_changed_store,
               GTK_TYPE_LIST_STORE)


static void
ggu_files_changed_store_class_init (GguFilesChangedStoreClass *klass)
{
  
}

static void
ggu_files_changed_store_init (GguFilesChangedStore *self)
{
  GType column_types[GGU_FILES_CHANGED_STORE_N_COLUMNS] = { 0 };
  
  column_types[GGU_FILES_CHANGED_STORE_COLUMN_ENTRY] = GGU_TYPE_GIT_FILES_CHANGED_ENTRY;
  
  gtk_list_store_set_column_types (GTK_LIST_STORE (self),
                                   G_N_ELEMENTS (column_types), column_types);
}


GguFilesChangedStore *
ggu_files_changed_store_new (void)
{
  return g_object_new (GGU_TYPE_FILES_CHANGED_STORE, NULL);
}

/**
 * ggu_files_changed_store_append:
 * @self: A #GguFilesChangedStore
 * @entry: (transfer none): The #GguGitFilesChangedEntry to append
 * 
 * Appends a new row and fills it with @entry.
 */
void
ggu_files_changed_store_append (GguFilesChangedStore    *self,
                                GguGitFilesChangedEntry *entry)
{
  GtkTreeIter iter;
  
  gtk_list_store_append (GTK_LIST_STORE (self), &iter);
  gtk_list_store_set (GTK_LIST_STORE (self), &iter,
                      GGU_FILES_CHANGED_STORE_COLUMN_ENTRY, entry, -1);
}

/**
 * ggu_files_changed_store_get_entry:
 * @self: A #GguFilesChangedStore
 * @iter: The #GtkTreeIter pointing to the row to get
 * 
 * Gets the #GguGitFilesChangedEntry at a given row.
 * 
 * Returns: (transfer none): The entry at the given row.
 */
GguGitFilesChangedEntry *
ggu_files_changed_store_get_entry (GguFilesChangedStore *self,
                                   GtkTreeIter          *iter)
{
  GguGitFilesChangedEntry *entry;
  
  gtk_tree_model_get (GTK_TREE_MODEL (self), iter,
                      GGU_FILES_CHANGED_STORE_COLUMN_ENTRY, &entry, -1);
  ggu_git_files_changed_entry_unref (entry);
  
  return entry;
}
