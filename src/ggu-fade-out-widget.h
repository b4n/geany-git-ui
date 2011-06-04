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

#ifndef H_GGU_FADE_OUT_WIDGET
#define H_GGU_FADE_OUT_WIDGET

#include <stdarg.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define GGU_TYPE_FADE_OUT_WIDGET            (ggu_fade_out_widget_get_type ())
#define GGU_FADE_OUT_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_FADE_OUT_WIDGET, GguFadeOutWidget))
#define GGU_FADE_OUT_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_FADE_OUT_WIDGET, GguFadeOutWidgetClass))
#define GGU_IS_FADE_OUT_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_FADE_OUT_WIDGET))
#define GGU_IS_FADE_OUT_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_FADE_OUT_WIDGET))
#define GGU_FADE_OUT_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_FADE_OUT_WIDGET, GguFadeOutWidgetClass))


typedef struct _GguFadeOutWidget        GguFadeOutWidget;
typedef struct _GguFadeOutWidgetClass   GguFadeOutWidgetClass;
typedef struct _GguFadeOutWidgetPrivate GguFadeOutWidgetPrivate;

struct _GguFadeOutWidget
{
  GtkDrawingArea parent_instance;
  GguFadeOutWidgetPrivate *priv;
};

struct _GguFadeOutWidgetClass
{
  GtkDrawingAreaClass parent_class;
};


GType         ggu_fade_out_widget_get_type            (void) G_GNUC_CONST;
GtkWidget    *ggu_fade_out_widget_new                 (GtkOrientation orientation,
                                                       GtkWidget     *original);


G_END_DECLS

#endif /* guard */
