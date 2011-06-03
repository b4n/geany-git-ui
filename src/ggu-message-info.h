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

#ifndef H_GGU_MESSAGE_INFO
#define H_GGU_MESSAGE_INFO

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GGU_TYPE_MESSAGE_INFO             (ggu_message_info_get_type())
#define GGU_MESSAGE_INFO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_MESSAGE_INFO, GguMessageInfo))
#define GGU_MESSAGE_INFO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_MESSAGE_INFO, GguMessageInfoClass))
#define GGU_IS_MESSAGE_INFO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_MESSAGE_INFO))
#define GGU_IS_MESSAGE_INFO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_MESSAGE_INFO))
#define GGU_MESSAGE_INFO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_MESSAGE_INFO, GguMessageInfoClass))


typedef struct _GguMessageInfo        GguMessageInfo;
typedef struct _GguMessageInfoClass   GguMessageInfoClass;
typedef struct _GguMessageInfoPrivate GguMessageInfoPrivate;

struct _GguMessageInfo
{
  GtkInfoBar parent_instance;
  GguMessageInfoPrivate *priv;
};

struct _GguMessageInfoClass
{
  GtkInfoBarClass parent_class;
};


GType         ggu_message_info_get_type                   (void) G_GNUC_CONST;
GtkWidget    *ggu_message_info_new                        (GtkMessageType type,
                                                           GtkButtonsType buttons,
                                                           const gchar   *format,
                                                           ...);
GtkWidget    *ggu_message_info_new_with_markup            (GtkMessageType type,
                                                           GtkButtonsType buttons,
                                                           const gchar   *format,
                                                           ...);
void          ggu_message_info_set_label                  (GguMessageInfo *self,
                                                           const gchar    *label);
const gchar  *ggu_message_info_get_label                  (GguMessageInfo *self);
void          ggu_message_info_set_use_markup             (GguMessageInfo *self,
                                                           gboolean        use_markup);
gboolean      ggu_message_info_get_use_markup             (GguMessageInfo *self);
void          ggu_message_info_set_secondary_label        (GguMessageInfo *self,
                                                           const gchar    *label);
const gchar  *ggu_message_info_get_secondary_label        (GguMessageInfo *self);
void          ggu_message_info_set_secondary_use_markup   (GguMessageInfo *self,
                                                           gboolean        use_markup);
gboolean      ggu_message_info_get_secondary_use_markup   (GguMessageInfo *self);
void          ggu_message_info_format_secondary_text      (GguMessageInfo *self,
                                                           const gchar    *format,
                                                           ...);
void          ggu_message_info_format_secondary_markup    (GguMessageInfo *self,
                                                           const gchar    *format,
                                                           ...);


G_END_DECLS

#endif /* guard */
