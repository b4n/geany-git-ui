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

#include "ggu-message-info.h"

#include "config.h"

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>


struct _GguMessageInfoPrivate
{
  GtkWidget  *icon;
  
  gboolean    primary_use_markup;
  GtkWidget  *primary_label;
  gboolean    secondary_use_markup;
  GtkWidget  *secondary_label;
};


static void     ggu_message_info_notify_message_type_handler  (GguMessageInfo *self,
                                                               GParamSpec     *pspec,
                                                               gpointer        data);
static void     ggu_message_info_get_propery                  (GObject    *object,
                                                               guint       prop_id,
                                                               GValue     *value,
                                                               GParamSpec *pspec);
static void     ggu_message_info_set_propery                  (GObject      *object,
                                                               guint         prop_id,
                                                               const GValue *value,
                                                               GParamSpec   *pspec);


G_DEFINE_TYPE (GguMessageInfo,
               ggu_message_info,
               GTK_TYPE_INFO_BAR)


enum
{
  PROP_0,
  
  PROP_BUTTONS,
  PROP_SECONDARY_TEXT,
  PROP_SECONDARY_USE_MARKUP,
  PROP_TEXT,
  PROP_USE_MARKUP
};


static void
ggu_message_info_finalize (GObject *object)
{
  GguMessageInfo *self = GGU_MESSAGE_INFO (object);
  
  G_OBJECT_CLASS (ggu_message_info_parent_class)->finalize (object);
}

static void
ggu_message_info_class_init (GguMessageInfoClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->finalize      = ggu_message_info_finalize;
  object_class->get_property  = ggu_message_info_get_propery;
  object_class->set_property  = ggu_message_info_set_propery;
  
  widget_class->show_all = gtk_widget_show;
  widget_class->hide_all = gtk_widget_hide;
  
  g_object_class_install_property (object_class, PROP_BUTTONS,
                                   g_param_spec_enum ("buttons",
                                                      "Buttons",
                                                      "The buttons shown in the message info",
                                                      GTK_TYPE_BUTTONS_TYPE,
                                                      GTK_BUTTONS_NONE,
                                                      G_PARAM_WRITABLE |
                                                      G_PARAM_CONSTRUCT_ONLY |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_SECONDARY_TEXT,
                                   g_param_spec_string ("secondary-text",
                                                        "Secondary text",
                                                        "The secondary text",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_SECONDARY_USE_MARKUP,
                                   g_param_spec_boolean ("secondary-use-markup",
                                                         "Secondary use markup",
                                                         "Whether the secondary text uses Pango markup",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "SText",
                                                        "The primary text",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_USE_MARKUP,
                                   g_param_spec_boolean ("use-markup",
                                                         "Use markup",
                                                         "Whether the primary text uses Pango markup",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguMessageInfoPrivate));
}

static void
ggu_message_info_init (GguMessageInfo *self)
{
  GtkWidget *content_area;
  GtkWidget *box;
  GtkWidget *label_box;
  
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_MESSAGE_INFO,
                                            GguMessageInfoPrivate);
  
  self->priv->icon = gtk_image_new ();
  self->priv->primary_label = g_object_new (GTK_TYPE_LABEL,
                                            "ellipsize", PANGO_ELLIPSIZE_END,
                                            "xalign", 0.0,
                                            NULL);
  self->priv->secondary_label = gtk_label_new (NULL);
  
  box = gtk_hbox_new (FALSE, 6);
  label_box = gtk_vbox_new (FALSE, 12);
  
  gtk_box_pack_start (GTK_BOX (box), self->priv->icon, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), label_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (label_box), self->priv->primary_label,
                      TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (label_box), self->priv->secondary_label,
                      TRUE, TRUE, 0);
  content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (self));
  gtk_box_pack_start (GTK_BOX (content_area), box, TRUE, TRUE, 0);
  
  g_signal_connect (self, "notify::message-type",
                    G_CALLBACK (ggu_message_info_notify_message_type_handler),
                    NULL);
  
  gtk_widget_show (box);
  gtk_widget_show (self->priv->icon);
  gtk_widget_show (label_box);
  gtk_widget_show (self->priv->primary_label);
}

static void
ggu_message_info_get_propery (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GguMessageInfo *self = GGU_MESSAGE_INFO (object);
  
  switch (prop_id) {
    case PROP_SECONDARY_TEXT:
      g_value_set_string (value, ggu_message_info_get_secondary_label (self));
      break;
    
    case PROP_SECONDARY_USE_MARKUP:
      g_value_set_boolean (value,
                           ggu_message_info_get_secondary_use_markup (self));
      break;
    
    case PROP_TEXT:
      g_value_set_string (value, ggu_message_info_get_label (self));
      break;
    
    case PROP_USE_MARKUP:
      g_value_set_boolean (value, ggu_message_info_get_use_markup (self));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_message_info_set_propery (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GguMessageInfo *self = GGU_MESSAGE_INFO (object);
  
  switch (prop_id) {
    case PROP_BUTTONS:
      switch (g_value_get_enum (value)) {
        case GTK_BUTTONS_CANCEL:
          gtk_info_bar_add_button (GTK_INFO_BAR (self),
                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
          break;
        
        case GTK_BUTTONS_CLOSE:
          gtk_info_bar_add_button (GTK_INFO_BAR (self),
                                   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
          break;
        
        case GTK_BUTTONS_OK:
          gtk_info_bar_add_button (GTK_INFO_BAR (self),
                                   GTK_STOCK_OK, GTK_RESPONSE_OK);
          break;
        
        case GTK_BUTTONS_OK_CANCEL:
          gtk_info_bar_add_buttons (GTK_INFO_BAR (self),
                                    GTK_STOCK_OK, GTK_RESPONSE_OK,
                                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                    NULL);
          break;
        
        case GTK_BUTTONS_YES_NO:
          gtk_info_bar_add_buttons (GTK_INFO_BAR (self),
                                    GTK_STOCK_YES, GTK_RESPONSE_YES,
                                    GTK_STOCK_NO, GTK_RESPONSE_NO,
                                    NULL);
          break;
        
        default: break;
      }
      break;
    
    case PROP_SECONDARY_TEXT:
      ggu_message_info_set_secondary_label (self, g_value_get_string (value));
      break;
    
    case PROP_SECONDARY_USE_MARKUP:
      ggu_message_info_set_secondary_use_markup (self,
                                                 g_value_get_boolean (value));
      break;
    
    case PROP_TEXT:
      ggu_message_info_set_label (self, g_value_get_string (value));
      break;
    
    case PROP_USE_MARKUP:
      ggu_message_info_set_use_markup (self, g_value_get_boolean (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_message_info_notify_message_type_handler (GguMessageInfo *self,
                                              GParamSpec     *pspec,
                                              gpointer        data)
{
  const gchar *stock;
  
  switch (gtk_info_bar_get_message_type (GTK_INFO_BAR (self))) {
    case GTK_MESSAGE_ERROR:     stock = GTK_STOCK_DIALOG_ERROR;     break;
    case GTK_MESSAGE_INFO:      stock = GTK_STOCK_DIALOG_INFO;      break;
    case GTK_MESSAGE_QUESTION:  stock = GTK_STOCK_DIALOG_QUESTION;  break;
    case GTK_MESSAGE_WARNING:   stock = GTK_STOCK_DIALOG_WARNING;   break;
    default:                    stock = NULL;                       break;
  }
  
  gtk_image_set_from_stock (GTK_IMAGE (self->priv->icon), stock,
                            GTK_ICON_SIZE_DIALOG);
}


GtkWidget *
ggu_message_info_new (GtkMessageType type,
                      GtkButtonsType buttons,
                      const gchar   *format,
                      ...)
{
  GtkWidget  *self;
  gchar      *text;
  va_list     ap;
  
  va_start (ap, format);
  text = g_strdup_vprintf (format, ap);
  va_end (ap);
  self = g_object_new (GGU_TYPE_MESSAGE_INFO,
                       "message-type", type,
                       "buttons", buttons,
                       "text", text,
                       NULL);
  g_free (text);
  
  return self;
}

GtkWidget *
ggu_message_info_new_with_markup (GtkMessageType type,
                                  GtkButtonsType buttons,
                                  const gchar   *format,
                                  ...)
{
  GtkWidget  *self;
  gchar      *text;
  va_list     ap;
  
  va_start (ap, format);
  text = g_markup_vprintf_escaped (format, ap);
  va_end (ap);
  self = g_object_new (GGU_TYPE_MESSAGE_INFO,
                       "message-type", type,
                       "buttons", buttons,
                       "text", text,
                       "use-makup", TRUE,
                       NULL);
  g_free (text);
  
  return self;
}

static void
update_label_tooltip (GguMessageInfo *self)
{
  GtkLabel *label = GTK_LABEL (self->priv->primary_label);
  
  if (gtk_label_get_use_markup (label)) {
    gtk_widget_set_tooltip_markup (self->priv->primary_label,
                                   gtk_label_get_label (label));
  } else {
    gtk_widget_set_tooltip_text (self->priv->primary_label,
                                 gtk_label_get_label (label));
  }
}

void
ggu_message_info_set_use_markup (GguMessageInfo *self,
                                 gboolean        use_markup)
{
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  if (gtk_label_get_use_markup (GTK_LABEL (self->priv->primary_label)) != use_markup) {
    gtk_label_set_use_markup (GTK_LABEL (self->priv->primary_label),
                              use_markup);
    update_label_tooltip (self);
    g_object_notify (G_OBJECT (self), "use-markup");
  }
}

gboolean
ggu_message_info_get_use_markup (GguMessageInfo *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_INFO (self), FALSE);
  
  return gtk_label_get_use_markup (GTK_LABEL (self->priv->primary_label));
}

void
ggu_message_info_set_label (GguMessageInfo *self,
                            const gchar    *label)
{
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  gtk_label_set_label (GTK_LABEL (self->priv->primary_label), label);
  update_label_tooltip (self);
}

const gchar *
ggu_message_info_get_label (GguMessageInfo *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_INFO (self), NULL);
  
  return gtk_label_get_label (GTK_LABEL (self->priv->primary_label));
}

void
ggu_message_info_set_secondary_use_markup (GguMessageInfo *self,
                                           gboolean        use_markup)
{
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  if (gtk_label_get_use_markup (GTK_LABEL (self->priv->secondary_label)) != use_markup) {
    gtk_label_set_use_markup (GTK_LABEL (self->priv->secondary_label),
                              use_markup);
    g_object_notify (G_OBJECT (self), "secondary-use-markup");
  }
}

gboolean
ggu_message_info_get_secondary_use_markup (GguMessageInfo *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_INFO (self), FALSE);
  
  return gtk_label_get_use_markup (GTK_LABEL (self->priv->secondary_label));
}

void
ggu_message_info_set_secondary_label (GguMessageInfo *self,
                                      const gchar    *label)
{
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  gtk_label_set_label (GTK_LABEL (self->priv->secondary_label), label);
  if (! gtk_label_get_use_markup (GTK_LABEL (self->priv->primary_label)) &&
      gtk_widget_get_visible (self->priv->secondary_label) != (label != NULL)) {
    if (! label) {
      gtk_label_set_attributes (GTK_LABEL (self->priv->primary_label), NULL);
    } else {
      PangoAttrList *list;
      
      /* make primary label bold */
      list = pango_attr_list_new ();
      pango_attr_list_insert (list, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
      gtk_label_set_attributes (GTK_LABEL (self->priv->primary_label), list);
      pango_attr_list_unref (list);
    }
  }
  gtk_widget_set_visible (self->priv->secondary_label, label != NULL);
  
  g_object_notify (G_OBJECT (self), "secondary-text");
}

const gchar *
ggu_message_info_get_secondary_label (GguMessageInfo *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_INFO (self), NULL);
  
  return gtk_label_get_label (GTK_LABEL (self->priv->secondary_label));
}

void
ggu_message_info_format_secondary_text (GguMessageInfo *self,
                                        const gchar    *format,
                                        ...)
{
  gchar  *text;
  va_list ap;
  
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  va_start (ap, format);
  text = g_strdup_vprintf (format, ap);
  va_end (ap);
  ggu_message_info_set_secondary_label (self, text);
  g_free (text);
}

void
ggu_message_info_format_secondary_markup (GguMessageInfo *self,
                                          const gchar    *format,
                                          ...)
{
  gchar  *markup;
  va_list ap;
  
  g_return_if_fail (GGU_IS_MESSAGE_INFO (self));
  
  va_start (ap, format);
  markup = g_markup_vprintf_escaped (format, ap);
  va_end (ap);
  ggu_message_info_set_secondary_label (self, markup);
  g_free (markup);
}
