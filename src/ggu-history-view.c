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

#include "ggu-history-view.h"

#include "config.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ggu-tree-view.h"
#include "ggu-history-store.h"


enum
{
  PROP_0,
  
  PROP_HASH_COLUMN_VISIBLE
};

struct _GguHistoryViewPrivate
{
  GtkTreeViewColumn  *hash_column;
  GtkTreeViewColumn  *summary_column;
};


static void       ggu_history_view_get_property               (GObject    *object,
                                                               guint       prop_id,
                                                               GValue     *value,
                                                               GParamSpec *pspec);
static void       ggu_history_view_set_property               (GObject      *object,
                                                               guint         prop_id,
                                                               const GValue *value,
                                                               GParamSpec   *pspec);
static void       ggu_history_view_populate_popup             (GguTreeView *view,
                                                               GtkTreePath *path,
                                                               GtkTreeIter *iter,
                                                               GtkMenu     *menu);
static gboolean   ggu_history_view_query_tooltip              (GtkWidget    *widget,
                                                               gint          x,
                                                               gint          y,
                                                               gboolean      keyboard_mode,
                                                               GtkTooltip   *tooltip);
static void       ggu_history_view_hash_cell_set_data_func    (GtkCellLayout   *cell_layout,
                                                               GtkCellRenderer *cell,
                                                               GtkTreeModel    *model,
                                                               GtkTreeIter     *iter,
                                                               gpointer         data);
static void       ggu_history_view_summary_cell_set_data_func (GtkCellLayout   *cell_layout,
                                                               GtkCellRenderer *cell,
                                                               GtkTreeModel    *model,
                                                               GtkTreeIter     *iter,
                                                               gpointer         data);


G_DEFINE_TYPE (GguHistoryView,
               ggu_history_view,
               GGU_TYPE_TREE_VIEW)


static void
ggu_history_view_class_init (GguHistoryViewClass *klass)
{
  GObjectClass     *object_class    = G_OBJECT_CLASS (klass);
  GtkWidgetClass   *widget_class    = GTK_WIDGET_CLASS (klass);
  GguTreeViewClass *tree_view_class = GGU_TREE_VIEW_CLASS (klass);
  
  object_class->get_property  = ggu_history_view_get_property;
  object_class->set_property  = ggu_history_view_set_property;
  
  widget_class->query_tooltip = ggu_history_view_query_tooltip;
  
  tree_view_class->populate_popup = ggu_history_view_populate_popup;
  
  g_object_class_install_property (object_class,
                                   PROP_HASH_COLUMN_VISIBLE,
                                   g_param_spec_boolean ("hash-column-visible",
                                                         "Hash column visible",
                                                         "Whether the hash column is visible",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguHistoryViewPrivate));
}

static void
ggu_history_view_init (GguHistoryView *self)
{
  GtkCellRenderer *cell;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_HISTORY_VIEW,
                                            GguHistoryViewPrivate);
  
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self), FALSE);
  gtk_widget_set_has_tooltip (GTK_WIDGET (self), TRUE);
  
  /* hash column */
  self->priv->hash_column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (self->priv->hash_column, _("Hash"));
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                       "style", PANGO_STYLE_ITALIC,
                       NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->hash_column),
                              cell, TRUE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (self->priv->hash_column),
                                      cell,
                                      ggu_history_view_hash_cell_set_data_func,
                                      NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self), self->priv->hash_column);
  
  /* summary column */
  self->priv->summary_column = gtk_tree_view_column_new ();
  gtk_tree_view_column_set_title (self->priv->summary_column, _("Summary"));
  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT,
                       "ellipsize", PANGO_ELLIPSIZE_END,
                       NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->summary_column),
                              cell, TRUE);
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (self->priv->summary_column),
                                      cell,
                                      ggu_history_view_summary_cell_set_data_func,
                                      NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self),
                               self->priv->summary_column);
}

static void
ggu_history_view_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GguHistoryView *self = GGU_HISTORY_VIEW (object);
  
  switch (prop_id) {
    case PROP_HASH_COLUMN_VISIBLE:
      g_value_set_boolean (value, ggu_history_view_get_hash_column_visible (self));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_history_view_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GguHistoryView *self = GGU_HISTORY_VIEW (object);
  
  switch (prop_id) {
    case PROP_HASH_COLUMN_VISIBLE:
      ggu_history_view_set_hash_column_visible (self, g_value_get_boolean (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
hash_column_visible_activate_handler (GtkMenuItem    *item,
                                      GguHistoryView *self)
{
  ggu_history_view_set_hash_column_visible (self,
                                            gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)));
}

static void
ggu_history_view_populate_popup (GguTreeView *self,
                                 GtkTreePath *path,
                                 GtkTreeIter *iter,
                                 GtkMenu     *menu)
{
  GtkWidget *item;
  
  /* show hash column */
  item = gtk_check_menu_item_new_with_mnemonic (_("Show _hash column"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                  ggu_history_view_get_hash_column_visible (GGU_HISTORY_VIEW (self)));
  g_signal_connect (item, "activate",
                    G_CALLBACK (hash_column_visible_activate_handler), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  gtk_widget_show (item);
}

static gboolean
ggu_history_view_query_tooltip (GtkWidget  *widget,
                                gint        x,
                                gint        y,
                                gboolean    keyboard_mode,
                                GtkTooltip *tooltip)
{
  GtkTreeModel   *model;
  GtkTreePath    *path;
  GtkTreeIter     iter;
  GguGitLogEntry *entry;
  gchar          *markup;
  
  if (! gtk_tree_view_get_tooltip_context (GTK_TREE_VIEW (widget),
                                           &x, &y, keyboard_mode,
                                           &model, &path, &iter)) {
    return FALSE;
  }
  
  entry = ggu_history_store_get_entry (GGU_HISTORY_STORE (model), &iter);
#if 1
  markup = g_markup_printf_escaped ("%s\n"
                                    "<small>"
                                    "<b>hash:</b>\t%s\n"
                                    "<b>date:</b>\t%s\n"
                                    "<b>author:</b>\t%s"
                                    "</small>",
                                    entry->summary, entry->hash, entry->date,
                                    entry->author);
#else
  markup = g_markup_escape_text (entry->summary, -1);
#endif
  
  gtk_tooltip_set_markup (tooltip, markup);
  gtk_tree_view_set_tooltip_row (GTK_TREE_VIEW (widget), tooltip, path);
  
  g_free (markup);
  gtk_tree_path_free (path);
  
  return TRUE;
}

/* function used to set the data of the cell renderer that renders the hash */
static void
ggu_history_view_hash_cell_set_data_func (GtkCellLayout   *cell_layout,
                                          GtkCellRenderer *cell,
                                          GtkTreeModel    *model,
                                          GtkTreeIter     *iter,
                                          gpointer         data)
{
  GguGitLogEntry *entry;
  GValue          value = { 0 };
  
  entry = ggu_history_store_get_entry (GGU_HISTORY_STORE (model), iter);
  g_value_init (&value, G_TYPE_STRING);
  g_value_take_string (&value, g_strndup (entry->hash, 7));
  g_object_set_property (G_OBJECT (cell), "text", &value);
  g_value_unset (&value);
}

static void
ggu_history_view_summary_cell_set_data_func (GtkCellLayout   *cell_layout,
                                             GtkCellRenderer *cell,
                                             GtkTreeModel    *model,
                                             GtkTreeIter     *iter,
                                             gpointer         data)
{
  GguGitLogEntry *entry;
  
  entry = ggu_history_store_get_entry (GGU_HISTORY_STORE (model), iter);
  g_object_set (G_OBJECT (cell), "text", entry->summary, NULL);
}


GtkWidget *
ggu_history_view_new (GguHistoryStore *model)
{
  return g_object_new (GGU_TYPE_HISTORY_VIEW,
                       "model", GTK_TREE_MODEL (model),
                       NULL);
}

gboolean
ggu_history_view_get_hash_column_visible (GguHistoryView *self)
{
  g_return_val_if_fail (GGU_IS_HISTORY_VIEW (self), FALSE);
  
  return gtk_tree_view_column_get_visible (self->priv->hash_column);
}

void
ggu_history_view_set_hash_column_visible (GguHistoryView *self,
                                          gboolean        visible)
{
  g_return_if_fail (GGU_IS_HISTORY_VIEW (self));
  
  if (gtk_tree_view_column_get_visible (self->priv->hash_column) != visible) {
    gtk_tree_view_column_set_visible (self->priv->hash_column, visible);
    g_object_notify (G_OBJECT (self), "hash-column-visible");
  }
}
