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

#ifndef H_GGU_MESSAGE_BOX
#define H_GGU_MESSAGE_BOX

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GGU_TYPE_MESSAGE_BOX            (ggu_message_box_get_type())
#define GGU_MESSAGE_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_MESSAGE_BOX, GguMessageBox))
#define GGU_MESSAGE_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_MESSAGE_BOX, GguMessageBoxClass))
#define GGU_IS_MESSAGE_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_MESSAGE_BOX))
#define GGU_IS_MESSAGE_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_MESSAGE_BOX))
#define GGU_MESSAGE_BOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_MESSAGE_BOX, GguMessageBoxClass))


typedef struct _GguMessageBox         GguMessageBox;
typedef struct _GguMessageBoxClass    GguMessageBoxClass;
typedef struct _GguMessageBoxPrivate  GguMessageBoxPrivate;

struct _GguMessageBox
{
  GtkBox parent_instance;
  GguMessageBoxPrivate *priv;
};

struct _GguMessageBoxClass
{
  GtkBoxClass parent_class;
};


GType         ggu_message_box_get_type            (void) G_GNUC_CONST;
GtkWidget    *ggu_message_box_new                 (GtkOrientation orirentation);
void          ggu_message_box_set_hard_limit      (GguMessageBox *self,
                                                   guint          hard_limit);
guint         ggu_message_box_get_hard_limit      (GguMessageBox *self);
void          ggu_message_box_set_soft_limit      (GguMessageBox *self,
                                                   guint          soft_limit);
guint         ggu_message_box_get_soft_limit      (GguMessageBox *self);


G_END_DECLS

#endif /* guard */
