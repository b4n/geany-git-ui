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

#ifndef H_GGU_HISTORY_VIEW
#define H_GGU_HISTORY_VIEW

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-history-store.h"

G_BEGIN_DECLS


#define GGU_TYPE_HISTORY_VIEW             (ggu_history_view_get_type ())
#define GGU_HISTORY_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_HISTORY_VIEW, GguHistoryView))
#define GGU_HISTORY_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_HISTORY_VIEW, GguHistoryViewClass))
#define GGU_IS_HISTORY_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_HISTORY_VIEW))
#define GGU_IS_HISTORY_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_HISTORY_VIEW))
#define GGU_HISTORY_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_HISTORY_VIEW, GguHistoryViewClass))


typedef struct _GguHistoryView        GguHistoryView;
typedef struct _GguHistoryViewClass   GguHistoryViewClass;
typedef struct _GguHistoryViewPrivate GguHistoryViewPrivate;

struct _GguHistoryView
{
  GtkTreeView parent_instance;
  GguHistoryViewPrivate *priv;
};

struct _GguHistoryViewClass
{
  GtkTreeViewClass parent_class;
  
  void      (*populate_popup)   (GguHistoryView *self,
                                 GtkTreePath    *path,
                                 GtkTreeIter    *iter,
                                 GtkMenu        *menu);
};


GType         ggu_history_view_get_type                     (void) G_GNUC_CONST;
GtkWidget    *ggu_history_view_new                          (GguHistoryStore *model);
gboolean      ggu_history_view_get_hash_column_visible      (GguHistoryView *self);
void          ggu_history_view_set_hash_column_visible      (GguHistoryView *self,
                                                             gboolean        visible);


G_END_DECLS

#endif /* guard */
