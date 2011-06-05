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

#include "ggu-marshal.h"
#include "ggu-history-store.h"


enum
{
  PROP_0,
  
  PROP_HASH_COLUMN_VISIBLE
};

enum
{
  SIGNAL_POPULATE_POPUP,
  
  N_SIGNALS
};

struct _GguHistoryViewPrivate
{
  GtkTreeViewColumn  *hash_column;
  GtkTreeViewColumn  *summary_column;
};


static void       ggu_history_view_get_propery                (GObject    *object,
                                                               guint       prop_id,
                                                               GValue     *value,
                                                               GParamSpec *pspec);
static void       ggu_history_view_set_propery                (GObject      *object,
                                                               guint         prop_id,
                                                               const GValue *value,
                                                               GParamSpec   *pspec);
static gboolean   ggu_history_view_button_press_event         (GtkWidget       *widget,
                                                               GdkEventButton  *event);
static gboolean   ggu_history_view_popup_menu                 (GtkWidget *widget);
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
               GTK_TYPE_TREE_VIEW)


guint signals[N_SIGNALS] = { 0 };


static void
ggu_history_view_class_init (GguHistoryViewClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->get_property  = ggu_history_view_get_propery;
  object_class->set_property  = ggu_history_view_set_propery;
  
  widget_class->button_press_event  = ggu_history_view_button_press_event;
  widget_class->popup_menu          = ggu_history_view_popup_menu;
  widget_class->query_tooltip       = ggu_history_view_query_tooltip;
  
  g_object_class_install_property (object_class,
                                   PROP_HASH_COLUMN_VISIBLE,
                                   g_param_spec_boolean ("hash-column-visible",
                                                         "Hash column visible",
                                                         "Whether the hash column is visible",
                                                         TRUE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  /**
   * GguHistotyView:populate-popup:
   * @self: the object that received the signal
   * @rev: the rev to show
   * 
   * This signal is emitted when the user activates the popup menu item to show
   * a particular revision content.
   */
  signals[SIGNAL_POPULATE_POPUP] = g_signal_new (
    "populate-popup",
    GGU_TYPE_HISTORY_VIEW,
    G_SIGNAL_RUN_LAST,
    G_STRUCT_OFFSET (GguHistoryViewClass, populate_popup),
    NULL, NULL,
    _ggu_cclosure_marshal_VOID__BOXED_BOXED_OBJECT,
    G_TYPE_NONE,
    3,
    GTK_TYPE_TREE_PATH,
    GTK_TYPE_TREE_ITER,
    GTK_TYPE_MENU);
  
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
ggu_history_view_get_propery (GObject    *object,
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
ggu_history_view_set_propery (GObject      *object,
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

static GtkWidget *
create_popup_menu (GguHistoryView  *self,
                   GtkTreePath     *path,
                   GtkTreeIter     *iter)
{
  GtkWidget *item;
  GtkWidget *menu;
  
  menu = gtk_menu_new ();
  
  /* show hash column */
  item = gtk_check_menu_item_new_with_mnemonic (_("Show _hash column"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                  ggu_history_view_get_hash_column_visible (self));
  g_signal_connect (item, "activate",
                    G_CALLBACK (hash_column_visible_activate_handler), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  
  /* let user code add items */
  g_signal_emit (self, signals[SIGNAL_POPULATE_POPUP], 0, path, iter, menu);
  
  gtk_widget_show_all (menu);
  
  return menu;
}

static gboolean
ggu_history_view_button_press_event (GtkWidget       *widget,
                                     GdkEventButton  *event)
{
  GtkTreeView  *view = GTK_TREE_VIEW (widget);
  gboolean      handled;
  
  /* run the default handler manually for it to set tup selection and stuff */
  handled = GTK_WIDGET_CLASS (ggu_history_view_parent_class)->button_press_event (widget, event);
  if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
    GtkTreePath  *path;
    GtkWidget    *menu;
    
    if (gtk_tree_view_get_path_at_pos (view, (gint) event->x, (gint) event->y,
                                       &path, NULL, NULL, NULL)) {
      GtkTreeIter iter;
      
      gtk_tree_model_get_iter (gtk_tree_view_get_model (view), &iter, path);
      menu = create_popup_menu (GGU_HISTORY_VIEW (widget), path, &iter);
      gtk_tree_path_free (path);
    } else {
      menu = create_popup_menu (GGU_HISTORY_VIEW (widget), NULL, NULL);
    }
    
    if (menu) {
      gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button,
                      event->time);
      handled = TRUE;
    }
  }
  
  return handled;
}

/* Position function for the popup menu.
 * It positions the popup below the selected row, or above if it doesn't fit.
 * If there is no selection, positions on the top left corner. */
static void
history_view_popup_menu_position_func (GtkMenu         *menu,
                                       gint            *x,
                                       gint            *y,
                                       gboolean        *push_in,
                                       GguHistoryView  *self)
{
  GtkTreeView      *view = GTK_TREE_VIEW (self);
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  
  gdk_window_get_origin (gtk_widget_get_window (GTK_WIDGET (self)), x, y);
  /* We let GTK do whatever it wants, so we just give a reasonable
   * suggestion without the need to check if we really end up with something
   * valuable (though we try hard) */
  *push_in = TRUE;
  
  selection = gtk_tree_view_get_selection (view);
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    GdkScreen      *screen = gtk_widget_get_screen (GTK_WIDGET (self));
    GtkTreePath    *path;
    GdkRectangle    rect;
    GtkRequisition  menu_req;
    
    gtk_widget_size_request (GTK_WIDGET (menu), &menu_req);
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_view_get_cell_area (view, path, NULL, &rect);
    gtk_tree_view_convert_bin_window_to_widget_coords (view, 0, rect.y,
                                                       NULL, &rect.y);
    gtk_tree_path_free (path);
    
    (*y) += rect.y + rect.height;
    /* If the menu doesn't fit below the row, try above */
    if ((*y) + menu_req.height > gdk_screen_get_height (screen)) {
      (*y) -= rect.height + menu_req.height;
    }
  } else {
    gtk_tree_view_convert_bin_window_to_widget_coords (view, 0, *y, NULL, y);
  }
}

static gboolean
ggu_history_view_popup_menu (GtkWidget *widget)
{
  GtkTreeView      *view = GTK_TREE_VIEW (widget);
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  GtkWidget        *menu;
  gboolean          handled = FALSE;
  
  selection = gtk_tree_view_get_selection (view);
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    GtkTreePath  *path;
    
    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_view_scroll_to_cell (view, path, NULL, FALSE, 0.0, 0.0);
    menu = create_popup_menu (GGU_HISTORY_VIEW (widget), path, &iter);
    gtk_tree_path_free (path);
  } else {
    menu = create_popup_menu (GGU_HISTORY_VIEW (widget), NULL, NULL);
  }
  if (menu) {
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                    (GtkMenuPositionFunc) history_view_popup_menu_position_func,
                    widget, 0, gtk_get_current_event_time ());
    handled = TRUE;
  }
  
  return handled;
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
