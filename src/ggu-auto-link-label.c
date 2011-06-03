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

#include "ggu-auto-link-label.h"

#include <glib.h>
#include <gtk/gtk.h>



static const struct {
  const gchar *match;
  const gchar *replacement;
} regexes[] = {
  { "(?:mailto:)?([^ \n\r\t\v<>&;\"]{2,}@[^ \n\r\t\v<>&;\"]{2,}\\.[a-zA-Z0-9]{2,})",
    "<a href=\"mailto:\\1\">\\0</a>" },
  { "(?<!<a href=\"mailto:)" /* don't match possible previous repacements */
    "((https?|sftp|ftps?|(git\\+)?ssh|samba|file|g?help):[^ \n\r\t\v]+)",
    "<a href=\"\\0\">\\0</a>" }
};

struct _GguAutoLinkLabelPrivate
{
  GRegex *regexes[2];
};


G_DEFINE_TYPE (GguAutoLinkLabel,
               ggu_auto_link_label,
               GTK_TYPE_LABEL)


static void
ggu_auto_link_label_finalize (GObject *object)
{
  GguAutoLinkLabel *self = GGU_AUTO_LINK_LABEL (object);
  guint             i;
  
  for (i = 0; i < G_N_ELEMENTS (self->priv->regexes); i++) {
    g_regex_unref (self->priv->regexes[i]);
    self->priv->regexes[i] = NULL;
  }
  
  G_OBJECT_CLASS (ggu_auto_link_label_parent_class)->finalize (object);
}

static void
ggu_auto_link_label_class_init (GguAutoLinkLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  object_class->finalize = ggu_auto_link_label_finalize;
  
  g_type_class_add_private (klass, sizeof (GguAutoLinkLabelPrivate));
}

static void
notify_label_handler (GguAutoLinkLabel *self,
                      GParamSpec       *pspec,
                      gpointer          data)
{
  const gchar *text;
  
  if (gtk_label_get_use_markup (GTK_LABEL (self))) {
    /* we can't currently handle input with links, so just do nothing for
     * marked-up inputs */
    return;
  }
  
  text = gtk_label_get_text (GTK_LABEL (self));
  if (text) {
    gchar *markup;
    guint  i;
    
    markup = g_markup_escape_text (text, -1);
    for (i = 0; i < G_N_ELEMENTS (self->priv->regexes); i++) {
      gchar  *tmp;
      GError *err = NULL;
    
      tmp = g_regex_replace (self->priv->regexes[i], markup, -1, 0,
                             regexes[i].replacement, 0, &err);
      if (! tmp) {
        g_critical ("Failed to preform regex replacement: %s", err->message);
        g_clear_error (&err);
      } else {
        g_free (markup);
        markup = tmp;
      }
    }
    gtk_label_set_markup (GTK_LABEL (self), markup);
    g_free (markup);
  }
}

static void
ggu_auto_link_label_init (GguAutoLinkLabel *self)
{
  guint i;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GGU_TYPE_AUTO_LINK_LABEL,
                                            GguAutoLinkLabelPrivate);
  
  for (i = 0; i < G_N_ELEMENTS (self->priv->regexes); i++) {
    GError *err = NULL;
    
    self->priv->regexes[i] = g_regex_new (regexes[i].match,
                                          G_REGEX_OPTIMIZE, 0, &err);
    if (! self->priv->regexes[i]) {
      g_critical ("Failed to build link matching regular expression \"%s\": %s",
                  regexes[i].match, err->message);
      g_error_free (err);
    }
  }
  g_signal_connect (self, "notify::label",
                    G_CALLBACK (notify_label_handler), NULL);
}


GtkWidget *
ggu_auto_link_label_new (const gchar *label)
{
  return g_object_new (GGU_TYPE_AUTO_LINK_LABEL, "label", label, NULL);
}
