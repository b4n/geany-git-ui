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

#ifndef H_GGU_WRAP_LABEL
#define H_GGU_WRAP_LABEL

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GGU_TYPE_WRAP_LABEL             (ggu_wrap_label_get_type())
#define GGU_WRAP_LABEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_WRAP_LABEL, GguWrapLabel))
#define GGU_WRAP_LABEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_WRAP_LABEL, GguWrapLabelClass))
#define GGU_IS_WRAP_LABEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_WRAP_LABEL))
#define GGU_IS_WRAP_LABEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_WRAP_LABEL))
#define GGU_WRAP_LABEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_WRAP_LABEL, GguWrapLabelClass))


typedef struct _GguWrapLabel        GguWrapLabel;
typedef struct _GguWrapLabelClass   GguWrapLabelClass;
typedef struct _GguWrapLabelPrivate GguWrapLabelPrivate;

struct _GguWrapLabel
{
  GtkLabel parent_instance;
  GguWrapLabelPrivate *priv;
};

struct _GguWrapLabelClass
{
  GtkLabelClass parent_class;
};


GType         ggu_wrap_label_get_type                      (void) G_GNUC_CONST;
GtkWidget    *ggu_wrap_label_new                           (const gchar *label);


G_END_DECLS

#endif /* guard */
