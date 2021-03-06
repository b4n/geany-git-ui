/* grind-plugin.c generated by valac 0.12.0.80-44805-dirty, the Vala compiler
 * generated from grind-plugin.vala, do not modify */

/*
 *  Copyright 2011 Colomban Wendling  <ban@herbesfolles.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*#include "ggu-plugin.h"*/

#include <glib.h>
#include <gtk/gtk.h>

#include "geanyplugin.h"
#include "document.h"

#include "ggu-git-utils.h"
#include "ggu-git-version.h"
#include "ggu-git-branch.h"
#include "ggu-panel.h"


GeanyPlugin    *geany_plugin;
GeanyData      *geany_data;
GeanyFunctions *geany_functions;


PLUGIN_VERSION_CHECK (206)
PLUGIN_SET_INFO ("Git UI",
                 "Git integration",
                 "0.1",
                 "Colomban Wendling <ban@herbesfolles.org>")


static GtkWidget *G_panel = NULL;

static void     document_activate_handler         (GObject       *geany_object,
                                                   GeanyDocument *doc,
                                                   gpointer       data);
static void     document_close_handler            (GObject       *geany_object,
                                                   GeanyDocument *doc,
                                                   gpointer       data);
static void     startup_complete_handler          (GObject       *geany_object,
                                                   gpointer       data);


PluginCallback plugin_callbacks[] = {
  { "document-activate",      G_CALLBACK (document_activate_handler), FALSE, NULL },
  { "document-open",          G_CALLBACK (document_activate_handler), FALSE, NULL },
  { "document-new",           G_CALLBACK (document_close_handler), FALSE, NULL },
  { "document-close",         G_CALLBACK (document_close_handler), FALSE, NULL },
  { "geany-startup-complete", G_CALLBACK (startup_complete_handler), FALSE, NULL },
  { NULL, NULL, FALSE, NULL }
};


static void
document_activate_handler (GObject       *object,
                           GeanyDocument *doc,
                           gpointer       data)
{
  ggu_panel_set_document (GGU_PANEL (G_panel), doc);
}

static void
document_close_handler (GObject       *object,
                        GeanyDocument *doc,
                        gpointer       data)
{
  if (doc == document_get_current ()) {
    ggu_panel_set_document (GGU_PANEL (G_panel), NULL);
  }
}

static void
check_version_async_finished_handler (GObject      *obj,
                                      GAsyncResult *result,
                                      gpointer      data)
{
  GError *err = NULL;
  
  if (! ggu_git_check_version_finish (GGU_GIT (obj), result, &err)) {
    GtkWidget *dialog;
    GtkWidget *content_box;
    GtkWidget *check;
    
    /* also log the standard way */
    g_warning ("Your Git version may be incompatible with this plugin: %s",
               err->message);
    
    /* FIXME: convert this to a MessageInfo */
    dialog = gtk_message_dialog_new (GTK_WINDOW (geany_data->main_widgets->window),
                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_CLOSE,
                                     "Your Git version may be incompatible with this plugin");
    g_object_set (dialog, "secondary-text", err->message, NULL);
    check = gtk_check_button_new_with_mnemonic (_("Don't show this warning again"));
    gtk_widget_show (check);
    content_box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_box_pack_start (GTK_BOX (content_box), check, FALSE, TRUE, 0);
    
    gtk_dialog_run (GTK_DIALOG (dialog));
    /* set_config (conf, "git", "show-version-warning",
                gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check))); */
    gtk_widget_destroy (dialog);
    
    g_error_free (err);
  }
}

static void
check_version (gint v1,
               gint v2,
               gint v3,
               gint v4)
{
  if (/*get_config_bool (conf, "git", "show-version-warning")*/ TRUE) {
    GguGit *git = GGU_GIT (ggu_git_branch_new ());
    
    ggu_git_check_version_async (git, v1, v2, v3, v4, NULL,
                                 check_version_async_finished_handler, NULL);
    g_object_unref (git);
  }
}

static void
startup_complete_handler (GObject  *geany_object,
                          gpointer  data)
{
  check_version (1, 7, 5, 0);
}

void
plugin_init (GeanyData *data)
{
  /* we register GTypes, we can't unload them */
  plugin_module_make_resident (geany_plugin);
  
  G_panel = ggu_panel_new ();
  gtk_notebook_append_page (GTK_NOTEBOOK (geany_data->main_widgets->sidebar_notebook),
                            G_panel, gtk_label_new (_("Git")));
  gtk_widget_show (G_panel);
  
  document_activate_handler (NULL, document_get_current (), NULL);
}

void
plugin_cleanup (void)
{
  gtk_widget_destroy (G_panel);
  G_panel = NULL;
}

/*GtkWidget *
plugin_configure (GtkDialog *dialog)
{
  
  return ;
}*/
