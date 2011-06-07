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

#ifndef H_GGU_FILES_CHANGED_STORE
#define H_GGU_FILES_CHANGED_STORE

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-git-files-changed-entry.h"

G_BEGIN_DECLS


#define GGU_TYPE_FILES_CHANGED_STORE            (ggu_files_changed_store_get_type ())
#define GGU_FILES_CHANGED_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_FILES_CHANGED_STORE, GguFilesChangedStore))
#define GGU_FILES_CHANGED_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_FILES_CHANGED_STORE, GguFilesChangedStoreClass))
#define GGU_IS_FILES_CHANGED_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_FILES_CHANGED_STORE))
#define GGU_IS_FILES_CHANGED_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_FILES_CHANGED_STORE))
#define GGU_FILES_CHANGED_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_FILES_CHANGED_STORE, GguFilesChangedStoreClass))


enum
{
  GGU_FILES_CHANGED_STORE_COLUMN_ENTRY,
  
  GGU_FILES_CHANGED_STORE_N_COLUMNS
};

typedef struct _GguFilesChangedStore       GguFilesChangedStore;
typedef struct _GguFilesChangedStoreClass  GguFilesChangedStoreClass;

struct _GguFilesChangedStore
{
  GtkListStore parent_instance;
};

struct _GguFilesChangedStoreClass
{
  GtkListStoreClass parent_class;
};


GType                     ggu_files_changed_store_get_type  (void) G_GNUC_CONST;
GguFilesChangedStore     *ggu_files_changed_store_new       (void);
void                      ggu_files_changed_store_append    (GguFilesChangedStore    *self,
                                                             GguGitFilesChangedEntry *entry);
GguGitFilesChangedEntry  *ggu_files_changed_store_get_entry (GguFilesChangedStore *self,
                                                             GtkTreeIter     *iter);


G_END_DECLS

#endif /* guard */
