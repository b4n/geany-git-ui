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

#include "ggu-files-changed-view.h"

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ggu-tree-view.h"
#include "ggu-files-changed-store.h"


#define ADDED_COLOR   "#008000"
#define REMOVED_COLOR "#C00000"


enum
{
  PROP_0,
  
  PROP_COLORIZE_CHANGES
};

struct _GguFilesChangedViewPrivate
{
  GtkTreeViewColumn  *path_column;
  GtkCellRenderer    *added_cell;
  GtkTreeViewColumn  *added_column;
  GtkCellRenderer    *removed_cell;
  GtkTreeViewColumn  *removed_column;
  gboolean            colorize_changes;
};


static void       ggu_files_changed_view_get_propery                (GObject    *object,
                                                                     guint       prop_id,
                                                                     GValue     *value,
                                                                     GParamSpec *pspec);
static void       ggu_files_changed_view_set_propery                (GObject      *object,
                                                                     guint         prop_id,
                                                                     const GValue *value,
                                                                     GParamSpec   *pspec);
static gboolean   ggu_files_changed_view_query_tooltip              (GtkWidget    *widget,
                                                                     gint          x,
                                                                     gint          y,
                                                                     gboolean      keyboard_mode,
                                                                     GtkTooltip   *tooltip);
static void       ggu_files_changed_view_path_cell_set_data_func    (GtkCellLayout   *cell_layout,
                                                                     GtkCellRenderer *cell,
                                                                     GtkTreeModel    *model,
                                                                     GtkTreeIter     *iter,
                                                                     gpointer         data);
static void       ggu_files_changed_view_added_cell_set_data_func   (GtkCellLayout   *cell_layout,
                                                                     GtkCellRenderer *cell,
                                                                     GtkTreeModel    *model,
                                                                     GtkTreeIter     *iter,
                                                                     gpointer         data);
static void       ggu_files_changed_view_removed_cell_set_data_func (GtkCellLayout   *cell_layout,
                                                                     GtkCellRenderer *cell,
                                                                     GtkTreeModel    *model,
                                                                     GtkTreeIter     *iter,
                                                                     gpointer         data);


G_DEFINE_TYPE (GguFilesChangedView,
               ggu_files_changed_view,
               GGU_TYPE_TREE_VIEW)


static void
ggu_files_changed_view_class_init (GguFilesChangedViewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->get_property  = ggu_files_changed_view_get_propery;
  object_class->set_property  = ggu_files_changed_view_set_propery;
  
  widget_class->query_tooltip = ggu_files_changed_view_query_tooltip;
  
  g_object_class_install_property (object_class,
                                   PROP_COLORIZE_CHANGES,
                                   g_param_spec_boolean ("colorize-changes",
                                                         "Colorize changes",
                                                         "Whether to colorize changes counts",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguFilesChangedViewPrivate));
}

static void
ggu_files_changed_view_init (GguFilesChangedView *self)
{
  GtkCellRenderer *cell;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_FILES_CHANGED_VIEW,
                                            GguFilesChangedViewPrivate);
  
  self->priv->colorize_changes = TRUE;
  
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self), FALSE);
  gtk_widget_set_has_tooltip (GTK_WIDGET (self), TRUE);
  
  /* path column */
  self->priv->path_column = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
                                          "title", _("Path"),
                                          "expand", TRUE,
                                          NULL);
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                       "ellipsize", PANGO_ELLIPSIZE_START,
                       NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->path_column),
                              cell, TRUE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (self->priv->path_column),
                                      cell,
                                      ggu_files_changed_view_path_cell_set_data_func,
                                      NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), self->priv->path_column);
  
  /* added column */
  self->priv->added_column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (self->priv->added_column, _("+"));
  self->priv->added_cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                                         "foreground", ADDED_COLOR,
                                         NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->added_column),
                              self->priv->added_cell, FALSE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (self->priv->added_column),
                                      self->priv->added_cell,
                                      ggu_files_changed_view_added_cell_set_data_func,
                                      NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), self->priv->added_column);
  
  /* removed column */
  self->priv->removed_column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (self->priv->removed_column, _("-"));
  self->priv->removed_cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                                           "foreground", REMOVED_COLOR,
                                           NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->removed_column),
                              self->priv->removed_cell, FALSE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (self->priv->removed_column),
                                      self->priv->removed_cell,
                                      ggu_files_changed_view_removed_cell_set_data_func,
                                      NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self),
                               self->priv->removed_column);
}

static void
ggu_files_changed_view_get_propery (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GguFilesChangedView *self = GGU_FILES_CHANGED_VIEW (object);
  
  switch (prop_id) {
    case PROP_COLORIZE_CHANGES:
      g_value_set_boolean (value, self->priv->colorize_changes);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_files_changed_view_set_propery (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GguFilesChangedView *self = GGU_FILES_CHANGED_VIEW (object);
  
  switch (prop_id) {
    case PROP_COLORIZE_CHANGES:
      ggu_files_changed_view_set_colorize_changes (self,
                                                   g_value_get_boolean (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
ggu_files_changed_view_query_tooltip (GtkWidget  *widget,
                                      gint        x,
                                      gint        y,
                                      gboolean    keyboard_mode,
                                      GtkTooltip *tooltip)
{
  GtkTreeModel             *model;
  GtkTreePath              *path;
  GtkTreeIter               iter;
  GguGitFilesChangedEntry  *entry;
  
  if (! gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget),
                                           &x, &y, keyboard_mode,
                                           &model, &path, &iter)) {
    return FALSE;
  }
  
  entry = ggu_files_changed_store_get_entry (GGU_FILES_CHANGED_STORE (model),
                                             &iter);
  
  gtk_tooltip_set_text (tooltip, entry->path);
  gtk_tree_view_set_tooltip_row (GTK_TREE_VIEW (widget), tooltip, path);
  gtk_tree_path_free (path);
  
  return TRUE;
}

/* function used to set the data of the cell renderer that renders the hash */
static void
ggu_files_changed_view_path_cell_set_data_func (GtkCellLayout   *cell_layout,
                                                GtkCellRenderer *cell,
                                                GtkTreeModel    *model,
                                                GtkTreeIter     *iter,
                                                gpointer         data)
{
  GguGitFilesChangedEntry  *entry;
  GValue                    value = { 0 };
  
  entry = ggu_files_changed_store_get_entry (GGU_FILES_CHANGED_STORE (model),
                                             iter);
  g_value_init (&value, G_TYPE_STRING);
  g_value_take_string (&value, g_strescape (entry->path, "\""));
  g_object_set_property (G_OBJECT (cell), "text", &value);
  g_value_unset (&value);
}

static void
ggu_files_changed_view_added_cell_set_data_func (GtkCellLayout   *cell_layout,
                                                 GtkCellRenderer *cell,
                                                 GtkTreeModel    *model,
                                                 GtkTreeIter     *iter,
                                                 gpointer         data)
{
  GguGitFilesChangedEntry  *entry;
  GValue                    value = { 0 };
  
  entry = ggu_files_changed_store_get_entry (GGU_FILES_CHANGED_STORE (model),
                                             iter);
  g_value_init (&value, G_TYPE_STRING);
  g_value_take_string (&value, g_strdup_printf ("+%u", entry->added));
  g_object_set_property (G_OBJECT (cell), "text", &value);
  g_value_unset (&value);
}

static void
ggu_files_changed_view_removed_cell_set_data_func (GtkCellLayout   *cell_layout,
                                                   GtkCellRenderer *cell,
                                                   GtkTreeModel    *model,
                                                   GtkTreeIter     *iter,
                                                   gpointer         data)
{
  GguGitFilesChangedEntry  *entry;
  GValue                    value = { 0 };
  
  entry = ggu_files_changed_store_get_entry (GGU_FILES_CHANGED_STORE (model),
                                             iter);
  g_value_init (&value, G_TYPE_STRING);
  g_value_take_string (&value, g_strdup_printf ("-%u", entry->removed));
  g_object_set_property (G_OBJECT (cell), "text", &value);
  g_value_unset (&value);
}


GtkWidget *
ggu_files_changed_view_new (GguFilesChangedStore *model)
{
  return g_object_new (GGU_TYPE_FILES_CHANGED_VIEW,
                       "model", GTK_TREE_MODEL (model),
                       NULL);
}


gboolean
ggu_files_changed_view_get_colorize_changes (GguFilesChangedView *self)
{
  g_return_val_if_fail (GGU_IS_FILES_CHANGED_VIEW (self), FALSE);
  
  return self->priv->colorize_changes;
}

void
ggu_files_changed_view_set_colorize_changes (GguFilesChangedView *self,
                                             gboolean             colorize)
{
  g_return_if_fail (GGU_IS_FILES_CHANGED_VIEW (self));
  
  if (self->priv->colorize_changes != colorize) {
    self->priv->colorize_changes = colorize;
    g_object_set (self->priv->added_cell,
                  "foreground-set", self->priv->colorize_changes,
                  NULL);
    g_object_set (self->priv->removed_cell,
                  "foreground-set", self->priv->colorize_changes,
                  NULL);
    g_object_notify (G_OBJECT (self), "colorize-changes");
  }
}
