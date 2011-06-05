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

#ifndef H_GGU_HISTORY_STORE
#define H_GGU_HISTORY_STORE

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-git-log-entry.h"

G_BEGIN_DECLS


#define GGU_TYPE_HISTORY_STORE            (ggu_history_store_get_type ())
#define GGU_HISTORY_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_HISTORY_STORE, GguHistoryStore))
#define GGU_HISTORY_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_HISTORY_STORE, GguHistoryStoreClass))
#define GGU_IS_HISTORY_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_HISTORY_STORE))
#define GGU_IS_HISTORY_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_HISTORY_STORE))
#define GGU_HISTORY_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_HISTORY_STORE, GguHistoryStoreClass))


enum
{
  GGU_HISTORY_STORE_COLUMN_ENTRY,
  
  GGU_HISTORY_STORE_N_COLUMNS
};

typedef struct _GguHistoryStore       GguHistoryStore;
typedef struct _GguHistoryStoreClass  GguHistoryStoreClass;

struct _GguHistoryStore
{
  GtkListStore parent_instance;
};

struct _GguHistoryStoreClass
{
  GtkListStoreClass parent_class;
};


GType             ggu_history_store_get_type          (void) G_GNUC_CONST;
GguHistoryStore  *ggu_history_store_new               (void);
void              ggu_history_store_append            (GguHistoryStore *self,
                                                       GguGitLogEntry  *entry);
GguGitLogEntry   *ggu_history_store_get_entry         (GguHistoryStore *self,
                                                       GtkTreeIter     *iter);


G_END_DECLS

#endif /* guard */
