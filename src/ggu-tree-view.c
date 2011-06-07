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

/*
 * A GtkTreeView that has a built-in popup menu.
 */

#include "ggu-tree-view.h"

#include <gtk/gtk.h>

#include "ggu-marshal.h"


enum
{
  SIGNAL_POPULATE_POPUP,
  
  N_SIGNALS
};


static gboolean   ggu_tree_view_button_press_event    (GtkWidget       *widget,
                                                       GdkEventButton  *event);
static gboolean   ggu_tree_view_popup_menu            (GtkWidget *widget);


G_DEFINE_ABSTRACT_TYPE (GguTreeView,
                        ggu_tree_view,
                        GTK_TYPE_TREE_VIEW)


static guint signals[N_SIGNALS] = { 0 };


static void
ggu_tree_view_class_init (GguTreeViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  widget_class->button_press_event  = ggu_tree_view_button_press_event;
  widget_class->popup_menu          = ggu_tree_view_popup_menu;
  
  /**
   * GguTreeView:populate-popup:
   * @self: The object that received the signal
   * @path: The GtkTreePath for which populate the menu, or %NULL if none
   * @iter: The GtkTreeIter for which populate the menu, or %NULL if none
   * @menu: The menu to pupulate
   * 
   * This signal is emitted when the popup menu should be populated.
   */
  signals[SIGNAL_POPULATE_POPUP] = g_signal_new (
    "populate-popup",
    GGU_TYPE_TREE_VIEW,
    G_SIGNAL_RUN_LAST,
    G_STRUCT_OFFSET (GguTreeViewClass, populate_popup),
    NULL, NULL,
    _ggu_cclosure_marshal_VOID__BOXED_BOXED_OBJECT,
    G_TYPE_NONE,
    3,
    GTK_TYPE_TREE_PATH,
    GTK_TYPE_TREE_ITER,
    GTK_TYPE_MENU);
}

static void
ggu_tree_view_init (GguTreeView *self)
{
  
}

static void
container_has_visible_child_callback (GtkWidget *widget,
                                      gpointer   data)
{
  if (gtk_widget_get_visible (widget)) {
    * (gboolean *) data = TRUE;
  }
}

static GtkWidget *
create_popup_menu (GguTreeView *self,
                   GtkTreePath *path,
                   GtkTreeIter *iter)
{
  GtkWidget  *menu;
  gboolean    has_visible_child = FALSE;
  
  menu = gtk_menu_new ();
  g_signal_emit (self, signals[SIGNAL_POPULATE_POPUP], 0, path, iter, menu);
  /* check if the menu has visible children, and don't popup if not */
  gtk_container_foreach (GTK_CONTAINER (menu),
                         container_has_visible_child_callback,
                         &has_visible_child);
  if (has_visible_child) {
    gtk_widget_show (menu);
  } else {
    gtk_widget_destroy (menu);
    menu = NULL;
  }
  
  return menu;
}

static gboolean
ggu_tree_view_button_press_event (GtkWidget      *widget,
                                  GdkEventButton *event)
{
  GtkTreeView  *view = GTK_TREE_VIEW (widget);
  gboolean      handled;
  
  /* run the default handler manually for it to set tup selection and stuff */
  handled = GTK_WIDGET_CLASS (ggu_tree_view_parent_class)->button_press_event (widget, event);
  if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
    GtkTreePath  *path;
    GtkWidget    *menu;
    
    if (gtk_tree_view_get_path_at_pos (view, (gint) event->x, (gint) event->y,
                                       &path, NULL, NULL, NULL)) {
      GtkTreeIter iter;
      
      gtk_tree_model_get_iter (gtk_tree_view_get_model (view), &iter, path);
      menu = create_popup_menu (GGU_TREE_VIEW (widget), path, &iter);
      gtk_tree_path_free (path);
    } else {
      menu = create_popup_menu (GGU_TREE_VIEW (widget), NULL, NULL);
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
tree_view_popup_menu_position_func (GtkMenu     *menu,
                                    gint        *x,
                                    gint        *y,
                                    gboolean    *push_in,
                                    GguTreeView *self)
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
ggu_tree_view_popup_menu (GtkWidget *widget)
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
    menu = create_popup_menu (GGU_TREE_VIEW (widget), path, &iter);
    gtk_tree_path_free (path);
  } else {
    menu = create_popup_menu (GGU_TREE_VIEW (widget), NULL, NULL);
  }
  if (menu) {
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                    (GtkMenuPositionFunc) tree_view_popup_menu_position_func,
                    widget, 0, gtk_get_current_event_time ());
    handled = TRUE;
  }
  
  return handled;
}
