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

#include "ggu-panel.h"

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-utils.h"
#include "ggu-plugin.h"
#include "git-lib/ggu-git-utils.h"
#include "git-lib/ggu-git-log.h"
#include "git-lib/ggu-git-log-entry.h"
#include "git-lib/ggu-git-files-changed-entry.h"
#include "git-lib/ggu-git-branch.h"
#include "git-lib/ggu-git-show.h"
#include "ggu-auto-link-label.h"
#include "ggu-files-changed-store.h"
#include "ggu-files-changed-view.h"
#include "ggu-history-store.h"
#include "ggu-history-view.h"
#include "ggu-message-box.h"
#include "ggu-message-info.h"

#include "geanyplugin.h"
#include "document.h"


static GtkWidget *
create_small_label (GType        type,
                    const gchar *text)
{
  GtkWidget     *label;
  PangoAttrList *attrs;
  
  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_SMALL));
  label = g_object_new (type,
                        "label", text,
                        "xalign", 0.0,
                        "ellipsize", PANGO_ELLIPSIZE_END,
                        "attributes", attrs,
                        "selectable", TRUE,
                        NULL);
  pango_attr_list_unref (attrs);
  
  return label;
}

static GtkWidget *
create_small_title_label (const gchar *text)
{
  GtkWidget     *label;
  PangoAttrList *attrs;
  
  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
  pango_attr_list_insert (attrs, pango_attr_scale_new (PANGO_SCALE_SMALL));
  label = g_object_new (GTK_TYPE_LABEL,
                        "label", text,
                        "xalign", 0.0,
                        "attributes", attrs,
                        NULL);
  pango_attr_list_unref (attrs);
  
  return label;
}

enum
{
  BRANCH_CURRENT,
  BRANCH_NAME,
  
  BRANCH_N_COLUMNS
};

struct _GguPanelPrivate
{
  GeanyDocument    *doc;
  gchar            *root;
  gchar            *path;
  
  guint             loading_count;
  
  GguGitLog        *logger;
  GCancellable     *log_cancellable;
  GguGitBranch     *brancher;
  GCancellable     *branch_cancellable;
  GguGitShow       *shower;
  GCancellable     *show_cancellable;
  GguGitShow       *changed_files_lister;
  GCancellable     *changed_files_list_cancellable;
  
  GtkWidget        *loading_spinner;
  GtkWidget        *file_path; /* FIXME: use a custom widget that shows repo root/current path */
  /* branch show/switch */
  GtkListStore     *branch_store;
  GtkWidget        *branch_combo;
  GtkWidget        *branch_switch;
  /* history */
  GguHistoryStore  *history_store;
  GtkWidget        *history_view;
  /* commit display */
  GtkWidget        *commit_container;
  GtkWidget        *commit_hash;
  GtkWidget        *commit_date;
  GtkWidget        *commit_author;
  GtkTextBuffer    *commit_message_buffer;
  GguFilesChangedStore *commit_files_changed_store;
  GtkWidget        *commit_files_changed_view;
  
  /* message displaying */
  GtkWidget        *message_area;
};


static void       ggu_panel_loading_push                    (GguPanel *self);
static void       ggu_panel_loading_pop                     (GguPanel *self);
static void       ggu_panel_show_rev                        (GguPanel      *self,
                                                             const gchar   *path,
                                                             const gchar   *rev,
                                                             gboolean       diff,
                                                             GeanyDocument *doc);
static void       ggu_panel_update_history                  (GguPanel    *self,
                                                             const gchar *rev);
static void       ggu_panel_update_branch_list              (GguPanel *self);
static void       ggu_panel_update_changed_files_list       (GguPanel    *self,
                                                             const gchar *rev);

static void       history_view_selection_changed_handler    (GtkTreeSelection *selection,
                                                             GguPanel         *self);
static void       history_view_populate_popup_handler       (GguHistoryView *view,
                                                             GtkTreePath    *path,
                                                             GtkTreeIter    *iter,
                                                             GtkMenu        *menu,
                                                             GguPanel       *self);
static void       branch_combo_changed_handler              (GtkComboBox *combo,
                                                             GguPanel    *self);
static void       files_changed_view_populate_popup_handler (GguFilesChangedView *view,
                                                             GtkTreePath         *path,
                                                             GtkTreeIter         *iter,
                                                             GtkMenu             *menu,
                                                             GguPanel            *self);
static void       files_changed_view_row_activated_handler  (GtkTreeView       *view,
                                                             GtkTreePath       *path,
                                                             GtkTreeViewColumn *column,
                                                             GguPanel          *self);


G_DEFINE_TYPE (GguPanel,
               ggu_panel,
               GTK_TYPE_VBOX)


static void
ggu_panel_finalize (GObject *object)
{
  GguPanel *self = GGU_PANEL (object);
  
  GGU_USPTR (self->priv->root);
  GGU_USPTR (self->priv->path);
  GGU_USOPTR (self->priv->logger);
  GGU_USOPTR (self->priv->log_cancellable);
  GGU_USOPTR (self->priv->brancher);
  GGU_USOPTR (self->priv->branch_cancellable);
  GGU_USOPTR (self->priv->shower);
  GGU_USOPTR (self->priv->show_cancellable);
  GGU_USOPTR (self->priv->changed_files_lister);
  GGU_USOPTR (self->priv->changed_files_list_cancellable);
  
  G_OBJECT_CLASS (ggu_panel_parent_class)->finalize (object);
}

static void
ggu_panel_class_init (GguPanelClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->finalize = ggu_panel_finalize;
  
  widget_class->show_all = gtk_widget_show;
  widget_class->hide_all = gtk_widget_hide;
  
  g_type_class_add_private (klass, sizeof (GguPanelPrivate));
}

static void
ggu_panel_init (GguPanel *self)
{
  GtkWidget          *paned;
  GtkWidget          *hbox;
  GtkWidget          *vbox;
  GtkWidget          *branch_box;
  GtkWidget          *table;
  GtkWidget          *scrolled;
  GtkWidget          *notebook;
  GtkWidget          *label;
  GtkCellRenderer    *cell;
  GtkTreeSelection   *selection;
  PangoAttrList      *attrs;
  GtkWidget          *commit_message_view;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_PANEL,
                                            GguPanelPrivate);
  
  self->priv->doc = NULL;
  self->priv->root = NULL;
  self->priv->path = NULL;
  self->priv->loading_count = 0;
  self->priv->logger = NULL;
  self->priv->log_cancellable = g_cancellable_new ();
  self->priv->brancher = NULL;
  self->priv->branch_cancellable = g_cancellable_new ();
  self->priv->shower = NULL;
  self->priv->show_cancellable = g_cancellable_new ();
  self->priv->changed_files_lister = NULL;
  self->priv->changed_files_list_cancellable = g_cancellable_new ();
  
  /* file path and spinner */
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (self), hbox, FALSE, TRUE, 0);
  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
  self->priv->file_path = g_object_new (GTK_TYPE_LABEL,
                                        "label", _("(none)"),
                                        "attributes", attrs,
                                        "xalign", 0.0,
                                        "ellipsize", PANGO_ELLIPSIZE_START,
                                        NULL);
  pango_attr_list_unref (attrs);
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->file_path, TRUE, TRUE, 0);
  self->priv->loading_spinner = gtk_spinner_new ();
  gtk_box_pack_start (GTK_BOX (hbox), self->priv->loading_spinner,
                      FALSE, TRUE, 0);
  
  /* the paned */
  paned = gtk_vpaned_new ();
  gtk_box_pack_start (GTK_BOX (self), paned, TRUE, TRUE, 0);
  
  /* the top pane content... */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_paned_pack1 (GTK_PANED (paned), vbox, TRUE, FALSE);
  
  /* branch show/switch */
  branch_box = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), branch_box, FALSE, TRUE, 0);
  label = gtk_label_new (_("Branch: "));
  gtk_box_pack_start (GTK_BOX (branch_box), label, FALSE, TRUE, 0);
  self->priv->branch_store = gtk_list_store_new (BRANCH_N_COLUMNS,
                                                 PANGO_TYPE_WEIGHT, /* bold if current */
                                                 G_TYPE_STRING      /* name */);
  self->priv->branch_combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (self->priv->branch_store));
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self->priv->branch_combo),
                              cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self->priv->branch_combo),
                                  cell,
                                  "weight", BRANCH_CURRENT,
                                  "text", BRANCH_NAME,
                                  NULL);
  g_signal_connect (self->priv->branch_combo, "changed",
                    G_CALLBACK (branch_combo_changed_handler), self);
  gtk_box_pack_start (GTK_BOX (branch_box), self->priv->branch_combo,
                      TRUE, TRUE, 0);
  self->priv->branch_switch = gtk_button_new_with_mnemonic (_("_Switch"));
  gtk_box_pack_start (GTK_BOX (branch_box), self->priv->branch_switch,
                      FALSE, TRUE, 0);
  
  /* the history view */
  scrolled = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "shadow-type", GTK_SHADOW_IN,
                           NULL);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled, TRUE, TRUE, 0);
  self->priv->history_store = ggu_history_store_new ();
  self->priv->history_view = ggu_history_view_new (self->priv->history_store);
  g_signal_connect_after (self->priv->history_view, "populate-popup",
                          G_CALLBACK (history_view_populate_popup_handler), self);
  gtk_container_add (GTK_CONTAINER (scrolled), self->priv->history_view);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->history_view));
  g_signal_connect (selection, "changed",
                    G_CALLBACK (history_view_selection_changed_handler), self);
  
  /* ...and the second pane content */
  self->priv->commit_container = gtk_vbox_new (FALSE, 0);
  gtk_paned_pack2 (GTK_PANED (paned), self->priv->commit_container,
                   FALSE, TRUE);
  
  /* commit hash, date and author */
  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (self->priv->commit_container), table,
                      FALSE, TRUE, 0);
  label = create_small_title_label (_("hash:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    GTK_FILL, GTK_FILL, 0, 0);
  self->priv->commit_hash = create_small_label (GTK_TYPE_LABEL, NULL);
  gtk_table_attach (GTK_TABLE (table), self->priv->commit_hash, 1, 2, 0, 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  label = create_small_title_label (_("date:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    GTK_FILL, GTK_FILL, 0, 0);
  self->priv->commit_date = create_small_label (GTK_TYPE_LABEL, NULL);
  gtk_table_attach (GTK_TABLE (table), self->priv->commit_date, 1, 2, 1, 2,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  label = create_small_title_label (_("author:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                    GTK_FILL, GTK_FILL, 0, 0);
  self->priv->commit_author = create_small_label (GGU_TYPE_AUTO_LINK_LABEL, NULL);
  gtk_table_attach (GTK_TABLE (table), self->priv->commit_author, 1, 2, 2, 3,
                    GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  
  /* commit details notebook (message, changes, etc.) */
  notebook = g_object_new (GTK_TYPE_NOTEBOOK,
                           "tab-pos", GTK_POS_BOTTOM,
                           NULL);
  gtk_box_pack_start (GTK_BOX (self->priv->commit_container), notebook,
                      TRUE, TRUE, 0);
  
  /* commit message */
  scrolled = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "shadow-type", GTK_SHADOW_IN,
                           NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            scrolled, gtk_label_new (_("Message")));
  self->priv->commit_message_buffer = gtk_text_buffer_new (NULL);
  commit_message_view = g_object_new (GTK_TYPE_TEXT_VIEW, 
                                      "buffer", self->priv->commit_message_buffer,
                                      "editable", FALSE,
                                      "wrap-mode", GTK_WRAP_WORD,
                                      NULL);
  gtk_container_add (GTK_CONTAINER (scrolled), commit_message_view);
  
  /* the files changed */
  scrolled = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
                           "shadow-type", GTK_SHADOW_IN,
                           NULL);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            scrolled, gtk_label_new (_("Changes")));
  self->priv->commit_files_changed_store = ggu_files_changed_store_new ();
  self->priv->commit_files_changed_view = ggu_files_changed_view_new (self->priv->commit_files_changed_store);
  g_signal_connect (self->priv->commit_files_changed_view, "populate-popup",
                    G_CALLBACK (files_changed_view_populate_popup_handler), self);
  g_signal_connect (self->priv->commit_files_changed_view, "row-activated",
                    G_CALLBACK (files_changed_view_row_activated_handler), self);
  gtk_container_add (GTK_CONTAINER (scrolled),
                     self->priv->commit_files_changed_view);
  
  /* info bar */
  /*self->priv->message_area = gtk_vbox_new (FALSE, 0);*/
  self->priv->message_area = g_object_new (GGU_TYPE_MESSAGE_BOX,
                                           "soft-limit", 1u,
                                           "hard-limit", 3u,
                                           "orientation", GTK_ORIENTATION_VERTICAL,
                                           NULL);
  gtk_box_pack_end (GTK_BOX (self), self->priv->message_area, FALSE, TRUE, 0);
  
  /* show children */
  gtk_widget_show_all (hbox);
  gtk_widget_show_all (paned);
  gtk_widget_show_all (self->priv->message_area);
  
  /* some defaults */
  gtk_widget_hide (self->priv->loading_spinner);
  gtk_widget_hide (self->priv->commit_container);
  gtk_widget_set_sensitive (self->priv->branch_switch, FALSE);
}

/* tells that a loading operation has started
 * if it is the first operation, reports to the user the loading started */
static void
ggu_panel_loading_push (GguPanel *self)
{
  if (self->priv->loading_count < 1) {
    gtk_spinner_start (GTK_SPINNER (self->priv->loading_spinner));
    gtk_widget_show (self->priv->loading_spinner);
  }
  self->priv->loading_count ++;
}

/* tells that a loading operation terminated
 * if it was the last operation, reports to the user the loading terminated */
static void
ggu_panel_loading_pop (GguPanel *self)
{
  g_return_if_fail (self->priv->loading_count > 0);
  
  self->priv->loading_count --;
  if (self->priv->loading_count < 1) {
    gtk_widget_hide (self->priv->loading_spinner);
    gtk_spinner_stop (GTK_SPINNER (self->priv->loading_spinner));
  }
}

static void
ggu_panel_set_git_path (GguPanel    *self,
                        const gchar *root,
                        const gchar *path)
{
  gchar *tooltip = NULL;
  
  GGU_SPTR (self->priv->root, g_strdup (root));
  GGU_SPTR (self->priv->path, g_strdup (path));
  
  if (! path) {
    path = _("(none)");
  } else {
    gchar    *color;
    GtkStyle *style;
    
    style = gtk_widget_get_style (self->priv->file_path);
    color = gdk_color_to_string (&style->text[GTK_STATE_INSENSITIVE]);
    tooltip = g_markup_printf_escaped ("<span color=\"%s\">%s</span><b>%s</b>",
                                       color, root, path);
    g_free (color);
  }
  gtk_label_set_text (GTK_LABEL (self->priv->file_path), path);
  gtk_widget_set_tooltip_markup (self->priv->file_path, tooltip);
  g_free (tooltip);
}

#define DOCUMENT_KEY  "ggu-geany-document"
#define LOG_ENTRY_KEY "ggu-log-entry"

static void
show_diff_activate_handler (GtkMenuItem *item,
                            GguPanel    *self)
{
  GguGitLogEntry *entry;
  
  entry = g_object_get_data (G_OBJECT (item), LOG_ENTRY_KEY);
  ggu_panel_show_rev (self, self->priv->path, entry->hash, TRUE, NULL);
}

static void
show_content_activate_handler (GtkMenuItem *item,
                               GguPanel    *self)
{
  GguGitLogEntry *entry;
  
  entry = g_object_get_data (G_OBJECT (item), LOG_ENTRY_KEY);
  ggu_panel_show_rev (self, self->priv->path, entry->hash, FALSE, NULL);
}

static void
replace_content_activate_handler (GtkMenuItem *item,
                                  GguPanel    *self)
{
  GguGitLogEntry *entry;
  
  entry = g_object_get_data (G_OBJECT (item), LOG_ENTRY_KEY);
  ggu_panel_show_rev (self, self->priv->path, entry->hash, FALSE,
                      self->priv->doc);
}

static GtkWidget *
history_view_create_popup_menu_item (GguPanel       *self,
                                     const gchar    *mnemonic,
                                     GguGitLogEntry *entry,
                                     void          (*callback) (GtkMenuItem *item,
                                                                GguPanel    *self))
{
  GtkWidget *item;
  
  item = gtk_image_menu_item_new_with_mnemonic (mnemonic);
  if (! entry) {
    gtk_widget_set_sensitive (item, FALSE);
  } else {
    g_object_set_data_full (G_OBJECT (item), LOG_ENTRY_KEY,
                            ggu_git_log_entry_ref (entry),
                            (GDestroyNotify) ggu_git_log_entry_unref);
    g_signal_connect (item, "activate", G_CALLBACK (callback), self);
  }
  
  return item;
}

static void
history_view_populate_popup_handler (GguHistoryView *view,
                                     GtkTreePath    *path,
                                     GtkTreeIter    *iter,
                                     GtkMenu        *menu,
                                     GguPanel       *self)
{
  GguGitLogEntry *entry = NULL;
  GtkWidget      *item;
  
  if (iter) {
    entry = ggu_history_store_get_entry (self->priv->history_store, iter);
  }

  /* <sep> */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
  /* show diff */
  item = history_view_create_popup_menu_item (self, _("Show _diff"), entry,
                                              show_diff_activate_handler);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* shiw content */
  item = history_view_create_popup_menu_item (self, _("Show _full content"),
                                              entry,
                                              show_content_activate_handler);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* <sep> */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
  /* replace content */
  item = history_view_create_popup_menu_item (self,
                                              _("_Replace document content"),
                                              entry,
                                              replace_content_activate_handler);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* <sep> */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
  /* general stuff */
  item = gtk_image_menu_item_new_with_mnemonic (_("Re_load repository"));
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                 gtk_image_new_from_stock (GTK_STOCK_REFRESH,
                                                           GTK_ICON_SIZE_MENU));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  
  gtk_widget_show_all (GTK_WIDGET (menu));
}

static void
history_view_selection_changed_handler (GtkTreeSelection *selection,
                                        GguPanel         *self)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    GguGitLogEntry *entry;
    
    entry = ggu_history_store_get_entry (GGU_HISTORY_STORE (model), &iter);
    
    ggu_panel_update_changed_files_list (self, entry->hash);
    
    gtk_label_set_text (GTK_LABEL (self->priv->commit_hash), entry->hash);
    gtk_label_set_text (GTK_LABEL (self->priv->commit_date), entry->date);
    gtk_label_set_text (GTK_LABEL (self->priv->commit_author), entry->author);
    gtk_text_buffer_set_text (self->priv->commit_message_buffer,
                              entry->details, -1);
    
    gtk_widget_show (self->priv->commit_container);
  } else {
    gtk_widget_hide (self->priv->commit_container);
  }
}

static void
branch_combo_changed_handler (GtkComboBox *combo,
                              GguPanel    *self)
{
  GtkTreeIter iter;
  
  gtk_list_store_clear (GTK_LIST_STORE (self->priv->history_store));
  gtk_widget_set_sensitive (self->priv->branch_switch, FALSE);
  if (gtk_combo_box_get_active_iter (combo, &iter)) {
    gchar  *branch;
    gint    weight;
    
    gtk_tree_model_get (gtk_combo_box_get_model (combo), &iter,
                        BRANCH_CURRENT, &weight,
                        BRANCH_NAME, &branch,
                        -1);
    if (weight == PANGO_WEIGHT_BOLD) {
      ggu_panel_update_history (self, NULL);
    } else {
      gtk_widget_set_sensitive (self->priv->branch_switch, TRUE);
      ggu_panel_update_history (self, branch);
    }
    g_free (branch);
  }
}


#define FILES_CHANGED_ENTRY_KEY "ggu-files-changed-entry"

static void
files_changed_view_use_colors_activate_handler (GtkMenuItem         *item,
                                                GguFilesChangedView *view)
{
  ggu_files_changed_view_set_colorize_changes (view,
                                               gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item)));
}

static void
files_changed_view_show_diff_activate_handler (GtkMenuItem *item,
                                               GguPanel    *self)
{
  GguGitFilesChangedEntry *entry = g_object_get_data (G_OBJECT (item),
                                                      FILES_CHANGED_ENTRY_KEY);
  
  ggu_panel_show_rev (self, entry->path, entry->hash, TRUE, NULL);
}

static void
files_changed_view_show_content_activate_handler (GtkMenuItem *item,
                                                  GguPanel    *self)
{
  GguGitFilesChangedEntry *entry = g_object_get_data (G_OBJECT (item),
                                                      FILES_CHANGED_ENTRY_KEY);
  
  ggu_panel_show_rev (self, entry->path, entry->hash, FALSE, NULL);
}

static void
files_changed_view_open_activate_handler (GtkMenuItem *item,
                                          GguPanel    *self)
{
  GguGitFilesChangedEntry *entry = g_object_get_data (G_OBJECT (item),
                                                      FILES_CHANGED_ENTRY_KEY);
  
  ggu_panel_open_repository_file (self, entry->path);
}

static GtkWidget *
files_changed_view_create_popup_menu_item (GguPanel                *self,
                                           const gchar             *mnemonic,
                                           GguGitFilesChangedEntry *entry,
                                           void                   (*callback) (GtkMenuItem *item,
                                                                               GguPanel    *self))
{
  GtkWidget *item;
  
  item = gtk_image_menu_item_new_with_mnemonic (mnemonic);
  if (! entry) {
    gtk_widget_set_sensitive (item, FALSE);
  } else {
    g_object_set_data_full (G_OBJECT (item), FILES_CHANGED_ENTRY_KEY,
                            ggu_git_files_changed_entry_ref (entry),
                            (GDestroyNotify) ggu_git_files_changed_entry_unref);
    g_signal_connect (item, "activate", G_CALLBACK (callback), self);
  }
  
  return item;
}

static void
files_changed_view_populate_popup_handler (GguFilesChangedView *view,
                                           GtkTreePath         *path,
                                           GtkTreeIter         *iter,
                                           GtkMenu             *menu,
                                           GguPanel            *self)
{
  GtkWidget                *item;
  GguGitFilesChangedEntry  *entry = NULL;
  
  if (iter) {
    entry = ggu_files_changed_store_get_entry (self->priv->commit_files_changed_store,
                                               iter);
  }
  
  /* whether to use colors */
  item = gtk_check_menu_item_new_with_mnemonic (_("Use colors"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item),
                                  ggu_files_changed_view_get_colorize_changes (view));
  g_signal_connect (item, "activate",
                    G_CALLBACK (files_changed_view_use_colors_activate_handler),
                    view);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* <sep> */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
  /* show diff */
  item = files_changed_view_create_popup_menu_item (self, _("Show _diff"),
                                                    entry,
                                                    files_changed_view_show_diff_activate_handler);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* show diff */
  item = files_changed_view_create_popup_menu_item (self, _("Show _full content"),
                                                    entry,
                                                    files_changed_view_show_content_activate_handler);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  /* <sep> */
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
  /* open file */
  item = files_changed_view_create_popup_menu_item (self, _("_Open file"),
                                                    entry,
                                                    files_changed_view_open_activate_handler);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item),
                                 gtk_image_new_from_stock (GTK_STOCK_OPEN,
                                                           GTK_ICON_SIZE_MENU));
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  
  gtk_widget_show_all (GTK_WIDGET (menu));
}

static void
files_changed_view_row_activated_handler (GtkTreeView       *view,
                                          GtkTreePath       *path,
                                          GtkTreeViewColumn *column,
                                          GguPanel          *self)
{
  GtkTreeIter               iter;
  GguGitFilesChangedEntry  *entry;
  GguFilesChangedStore     *store = self->priv->commit_files_changed_store;
  
  gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
  entry = ggu_files_changed_store_get_entry (store, &iter);
  ggu_panel_show_rev (self, entry->path, entry->hash, TRUE, NULL);
}

static void
ggu_panel_show_message_va (GguPanel       *self,
                           GtkMessageType  type,
                           const gchar    *primary,
                           const gchar    *fmt,
                           va_list         ap)
{
  GtkWidget  *info;
  gchar      *message;
  
  message = g_strdup_vprintf (fmt, ap);
  g_debug ("%s: %s", primary, message);
  info = g_object_new (GGU_TYPE_MESSAGE_INFO,
                       "message-type", type,
                       "buttons", GTK_BUTTONS_CLOSE,
                       "text", primary,
                       "secondary-text", message,
                       NULL);
  g_free (message);
  g_signal_connect (info, "response", G_CALLBACK (gtk_widget_destroy), NULL);
  
  gtk_container_add (GTK_CONTAINER (self->priv->message_area), info);
  gtk_widget_show (info);
}

static void
ggu_panel_show_message (GguPanel      *self,
                        GtkMessageType type,
                        const gchar   *primary,
                        const gchar   *fmt,
                        ...)
{
  va_list ap;
  
  va_start (ap, fmt);
  ggu_panel_show_message_va (self, type, primary, fmt, ap);
  va_end (ap);
}


GtkWidget *
ggu_panel_new (void)
{
  return g_object_new (GGU_TYPE_PANEL, NULL);
}

static void
ggu_panel_show_rev_async_finished_handler (GObject      *object,
                                           GAsyncResult *result,
                                           gpointer      data)
{
  GguPanel     *self = data;
  const gchar  *content;
  GError       *error = NULL;
  
  /* unconditionally pop our loading ref */
  ggu_panel_loading_pop (self);
  
  /* make sure it's the result of the last operation and not a previous
   * (possibly cancelled) one that terminates maybe after */
  if (GGU_GIT_SHOW (object) != self->priv->shower) {
    return;
  }
  
  content = ggu_git_show_show_finish (GGU_GIT_SHOW (object), result, &error);
  if (error) {
    if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
      ggu_panel_show_message (self, GTK_MESSAGE_ERROR,
                              "Show failed", "%s", error->message);
    }
    g_error_free (error);
  } else {
    GeanyDocument *doc;
    gboolean       diff;
    gchar         *rev;
    gchar         *path;
    gchar         *filename;
    gchar         *document_name;
    GeanyFiletype *ft = NULL;
    
    doc = g_object_get_data (object, DOCUMENT_KEY);
    g_return_if_fail (DOC_VALID (self->priv->doc));
    g_return_if_fail (doc == NULL || DOC_VALID (doc));
    
    g_object_get (object, "diff", &diff, "rev", &rev, "file", &path, NULL);
    filename = g_path_get_basename (path);
    if (diff) {
      ft = filetypes[GEANY_FILETYPES_DIFF];
      document_name = g_strdup_printf (_("Diff of %s at revision %.7s"),
                                       filename, rev);
    } else {
      ft = self->priv->doc->file_type;
      document_name = g_strdup_printf (_("%s at revision %.7s"), filename, rev);
    }
    
    if (! doc) {
      doc = document_new_file (document_name, ft, content);
    } else {
      sci_set_text (doc->editor->sci, content);
      document_set_filetype (doc, ft);
    }
    //sci_set_readonly (doc->editor->sci, TRUE);
    
    g_free (rev);
    g_free (path);
    g_free (filename);
    g_free (document_name);
  }
}

static void
ggu_panel_show_rev (GguPanel       *self,
                    const gchar    *path,
                    const gchar    *rev,
                    gboolean        diff,
                    GeanyDocument  *doc)
{
  g_cancellable_cancel (self->priv->show_cancellable);
  
  GGU_SOPTR (self->priv->shower, ggu_git_show_new ());
  g_object_set_data (G_OBJECT (self->priv->shower), DOCUMENT_KEY, doc);
  g_cancellable_reset (self->priv->show_cancellable);
  ggu_panel_loading_push (self);
  ggu_git_show_show_async (self->priv->shower,
                           self->priv->root, rev, path, diff,
                           self->priv->show_cancellable,
                           ggu_panel_show_rev_async_finished_handler, self);
}

static void
ggu_panel_update_history_async_finished_handler (GObject      *object,
                                                 GAsyncResult *result,
                                                 gpointer      data)
{
  GguPanel *self = data;
  GList    *entries;
  GError   *error = NULL;
  
  /* unconditionally pop our loading ref */
  ggu_panel_loading_pop (self);
  
  /* make sure it's the result of the last operation and not a previous
   * (possibly cancelled) one that terminates maybe after */
  if (GGU_GIT_LOG (object) != self->priv->logger) {
    return;
  }
  
  entries = ggu_git_log_log_finish (GGU_GIT_LOG (object), result, &error);
  if (error) {
    if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
      ggu_panel_show_message (self, GTK_MESSAGE_ERROR,
                              "History update failed", "%s", error->message);
    }
    g_error_free (error);
  } else {
    for (; entries; entries = entries->next) {
      ggu_history_store_append (self->priv->history_store, entries->data);
    }
  }
}

static void
ggu_panel_update_history (GguPanel    *self,
                          const gchar *rev)
{
  g_cancellable_cancel (self->priv->log_cancellable);
  gtk_list_store_clear (GTK_LIST_STORE (self->priv->history_store));
  
  GGU_SOPTR (self->priv->logger, ggu_git_log_new ());
  g_cancellable_reset (self->priv->log_cancellable);
  ggu_panel_loading_push (self);
  ggu_git_log_log_async (self->priv->logger,
                         self->priv->root, rev, self->priv->path,
                         self->priv->log_cancellable,
                         ggu_panel_update_history_async_finished_handler, self);
}

static void
ggu_panel_update_branch_list_async_finished_handler (GObject      *object,
                                                     GAsyncResult *result,
                                                     gpointer      data)
{
  GguPanel     *self = data;
  GList        *branches;
  const gchar  *current_branch;
  GError       *error = NULL;
  
  /* unconditionally pop our loading ref */
  ggu_panel_loading_pop (self);
  
  /* make sure it's the result of the last operation and not a previous
   * (possibly cancelled) one that terminates maybe after */
  if (GGU_GIT_BRANCH (object) != self->priv->brancher) {
    return;
  }
  
  branches = ggu_git_branch_list_finish (GGU_GIT_BRANCH (object),
                                         &current_branch, result, &error);
  if (error) {
    if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
      ggu_panel_show_message (self, GTK_MESSAGE_ERROR,
                              "Branch list failed", "%s", error->message);
    }
    g_error_free (error);
  } else {
    for (; branches; branches = branches->next) {
      GtkTreeIter iter;
      gboolean    current = branches->data == current_branch;
      
      gtk_list_store_append (self->priv->branch_store, &iter);
      gtk_list_store_set (self->priv->branch_store, &iter,
                          BRANCH_NAME, branches->data,
                          BRANCH_CURRENT, current ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
                          -1);
      if (current) {
        gtk_combo_box_set_active_iter (GTK_COMBO_BOX (self->priv->branch_combo),
                                       &iter);
      }
    }
  }
}

static void
ggu_panel_update_branch_list (GguPanel *self)
{
  g_cancellable_cancel (self->priv->branch_cancellable);
  gtk_list_store_clear (self->priv->branch_store);
  
  GGU_SOPTR (self->priv->brancher, ggu_git_branch_new ());
  g_cancellable_reset (self->priv->branch_cancellable);
  ggu_panel_loading_push (self);
  ggu_git_branch_list_async (self->priv->brancher, self->priv->root,
                             self->priv->branch_cancellable,
                             ggu_panel_update_branch_list_async_finished_handler,
                             self);
}

static void
ggu_panel_update_changed_files_list_async_finished_handler (GObject      *object,
                                                            GAsyncResult *result,
                                                            gpointer      data)
{
  GguPanel *self = data;
  GList    *entries;
  GError   *error = NULL;
  
  /* unconditionally pop our loading ref */
  ggu_panel_loading_pop (self);
  
  /* make sure it's the result of the last operation and not a previous
   * (possibly cancelled) one that terminates maybe after */
  if (GGU_GIT_SHOW (object) != self->priv->changed_files_lister) {
    return;
  }
  
  entries = ggu_git_list_files_changed_finish (GGU_GIT_SHOW (object),
                                               result, &error);
  if (error) {
    if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED) {
      ggu_panel_show_message (self, GTK_MESSAGE_ERROR,
                              "Files changed list failed",
                              "%s", error->message);
    }
    g_error_free (error);
  } else {
    for (; entries; entries = entries->next) {
      ggu_files_changed_store_append (self->priv->commit_files_changed_store,
                                      entries->data);
    }
  }
}

static void
ggu_panel_update_changed_files_list (GguPanel    *self,
                                     const gchar *rev)
{
  g_cancellable_cancel (self->priv->changed_files_list_cancellable);
  gtk_list_store_clear (GTK_LIST_STORE (self->priv->commit_files_changed_store));
  
  GGU_SOPTR (self->priv->changed_files_lister,  ggu_git_show_new ());
  g_cancellable_reset (self->priv->changed_files_list_cancellable);
  ggu_panel_loading_push (self);
  ggu_git_list_files_changed_async (self->priv->changed_files_lister,
                                    self->priv->root,
                                    rev,
                                    self->priv->changed_files_list_cancellable,
                                    ggu_panel_update_changed_files_list_async_finished_handler,
                                    self);
}

gboolean
ggu_panel_open_repository_file (GguPanel    *self,
                                const gchar *intern_path)
{
  gboolean  success;
  gchar    *path;
  gchar    *locale_path;
  
  path = g_strconcat (self->priv->root, intern_path, NULL);
  locale_path = g_filename_from_utf8 (path, -1, NULL, NULL, NULL);
  if (! locale_path) {
    locale_path = path;
  } else {
    g_free (path);
  }
  success = document_open_file (locale_path, FALSE, NULL, NULL) != NULL;
  g_free (locale_path);
  
  return success;
}

void
ggu_panel_set_document (GguPanel       *self,
                        GeanyDocument  *doc)
{
  gchar        *root = NULL;
  gchar        *inner_path = NULL;
  gboolean      is_valid;
  const gchar  *path;
  
  g_return_if_fail (GGU_IS_PANEL (self));
  
  if (DOC_VALID (doc) && self->priv->doc == doc) {
    /* no need to update */
    return;
  }
  
  self->priv->doc = DOC_VALID (doc) ? doc : NULL;
  path = self->priv->doc ? self->priv->doc->file_name : NULL;
  
  is_valid = ggu_git_parse_path (path, &root, &inner_path);
  gtk_widget_set_sensitive (GTK_WIDGET (self), is_valid);
  ggu_panel_set_git_path (self, root, inner_path);
  if (! is_valid) {
    /* cancel possible running operation */
    g_cancellable_cancel (self->priv->log_cancellable);
    g_cancellable_cancel (self->priv->branch_cancellable);
    
    gtk_list_store_clear (GTK_LIST_STORE (self->priv->history_store));
    gtk_list_store_clear (self->priv->branch_store);
  } else {
    /*ggu_panel_update_history (self, root, NULL, inner_path);*/
    ggu_panel_update_branch_list (self);
  }
  g_free (root);
  g_free (inner_path);
}
