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

#ifndef H_GGU_FILES_CHANGED_VIEW
#define H_GGU_FILES_CHANGED_VIEW

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-tree-view.h"
#include "ggu-files-changed-store.h"

G_BEGIN_DECLS


#define GGU_TYPE_FILES_CHANGED_VIEW             (ggu_files_changed_view_get_type ())
#define GGU_FILES_CHANGED_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_FILES_CHANGED_VIEW, GguFilesChangedView))
#define GGU_FILES_CHANGED_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_FILES_CHANGED_VIEW, GguFilesChangedViewClass))
#define GGU_IS_FILES_CHANGED_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_FILES_CHANGED_VIEW))
#define GGU_IS_FILES_CHANGED_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_FILES_CHANGED_VIEW))
#define GGU_FILES_CHANGED_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_FILES_CHANGED_VIEW, GguFilesChangedViewClass))


typedef struct _GguFilesChangedView         GguFilesChangedView;
typedef struct _GguFilesChangedViewClass    GguFilesChangedViewClass;
typedef struct _GguFilesChangedViewPrivate  GguFilesChangedViewPrivate;

struct _GguFilesChangedView
{
  GguTreeView parent_instance;
  GguFilesChangedViewPrivate *priv;
};

struct _GguFilesChangedViewClass
{
  GguTreeViewClass parent_class;
};


GType         ggu_files_changed_view_get_type             (void) G_GNUC_CONST;
GtkWidget    *ggu_files_changed_view_new                  (GguFilesChangedStore *model);
gboolean      ggu_files_changed_view_get_colorize_changes (GguFilesChangedView *self);
void          ggu_files_changed_view_set_colorize_changes (GguFilesChangedView *self,
                                                           gboolean             colorize);


G_END_DECLS

#endif /* guard */
