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

/**
 * #GguFadeOutWidget is a widget that makes animating another widget for fade
 * out easy. It takes a snapshot of the given widget and animates a fade out
 * with it, destroying itself when the animation finished (thus when the
 * widget disappeared).
 * 
 * The fact a snapshot is taken has pro and cons:
 *  * The animation will not be updated to new state of the widget. For
 *    example, the widget needs to be visible when creating a #GguFadeOutWidget
 *    from it.
 *  * The widget may be destroyed after the #GguFadeOutWidget is created,
 *    making easy to add animation on widget destruction.
 */

#include "ggu-fade-out-widget.h"

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-glib-compat.h"
#include "ggu-gtk-compat.h"
#include "ggu-utils.h"


#define FRAME_DURATION 30


struct _GguFadeOutWidgetPrivate
{
  GtkOrientation  orientation;
  GdkPixbuf      *pixbuf;
  gint            pixbuf_width;
  gint            pixbuf_height;
  GSource        *source;
  
  gdouble         factor;
  gdouble         width;
  gdouble         height;
};


static void     ggu_fade_out_widget_finalize                  (GObject *object);
static void     ggu_fade_out_widget_get_propery               (GObject    *object,
                                                               guint       prop_id,
                                                               GValue     *value,
                                                               GParamSpec *pspec);
static void     ggu_fade_out_widget_set_propery               (GObject      *object,
                                                               guint         prop_id,
                                                               const GValue *value,
                                                               GParamSpec   *pspec);
static gboolean ggu_fade_out_widget_expose_event              (GtkWidget      *widget,
                                                               GdkEventExpose *event);
static void     ggu_fade_out_widget_size_request              (GtkWidget      *widget,
                                                               GtkRequisition *requisition);
static void     ggu_fade_out_widget_show                      (GtkWidget *widget);


G_DEFINE_TYPE_WITH_CODE (GguFadeOutWidget,
                         ggu_fade_out_widget,
                         GTK_TYPE_DRAWING_AREA,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE,
                                                NULL))


enum
{
  PROP_0,
  
  PROP_ORIENTATION,
  PROP_WIDGET
};


static void
ggu_fade_out_widget_finalize (GObject *object)
{
  GguFadeOutWidget *self = GGU_FADE_OUT_WIDGET (object);
  
  GGU_USOPTR (self->priv->pixbuf);
  if (self->priv->source) {
    g_source_destroy (self->priv->source);
    self->priv->source = NULL;
  }
  
  G_OBJECT_CLASS (ggu_fade_out_widget_parent_class)->finalize (object);
}

static void
ggu_fade_out_widget_class_init (GguFadeOutWidgetClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  object_class->finalize      = ggu_fade_out_widget_finalize;
  object_class->get_property  = ggu_fade_out_widget_get_propery;
  object_class->set_property  = ggu_fade_out_widget_set_propery;
  
  widget_class->expose_event  = ggu_fade_out_widget_expose_event;
  widget_class->size_request  = ggu_fade_out_widget_size_request;
  widget_class->show          = ggu_fade_out_widget_show;
  
  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");
  
  g_object_class_install_property (object_class,
                                   PROP_WIDGET,
                                   g_param_spec_object ("widget",
                                                        "Widget",
                                                        "The widget to replicate",
                                                        GTK_TYPE_WIDGET,
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));
  
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_uint ("anim-duration",
                                                              "Anim duration",
                                                              "The animation durtaion, in milliseconds",
                                                              0,
                                                              G_MAXUINT,
                                                              500,
                                                              G_PARAM_READWRITE |
                                                              G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguFadeOutWidgetPrivate));
}

static void
ggu_fade_out_widget_init (GguFadeOutWidget *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_FADE_OUT_WIDGET,
                                            GguFadeOutWidgetPrivate);
  
  self->priv->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->priv->pixbuf = NULL;
  self->priv->pixbuf_width = 0;
  self->priv->pixbuf_height = 0;
  self->priv->source = NULL;
  self->priv->factor = 2.0;
  self->priv->width = 0.0;
  self->priv->height = 0.0;
}

static void
ggu_fade_out_widget_get_propery (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GguFadeOutWidget *self = GGU_FADE_OUT_WIDGET (object);
  
  switch (prop_id) {
    case PROP_ORIENTATION:
      g_value_set_enum (value, self->priv->orientation);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_fade_out_widget_set_propery (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GguFadeOutWidget *self = GGU_FADE_OUT_WIDGET (object);
  
  switch (prop_id) {
    case PROP_ORIENTATION: {
      GtkOrientation or = g_value_get_enum (value);
      
      if (or != self->priv->orientation) {
        self->priv->orientation = or;
        g_object_notify_by_pspec (G_OBJECT (self), pspec);
      }
    } break;
    
    case PROP_WIDGET: {
      GtkWidget  *widget = g_value_get_object (value);
      
      if (! gtk_widget_get_realized (widget)) {
        g_warning ("A GguFadeOutWidget can only be created from a realized "
                   "widget");
      } else {
        GdkPixmap  *pixmap;
        
        pixmap = gtk_widget_get_snapshot (widget, NULL);
        gdk_pixmap_get_size (pixmap,
                             &self->priv->pixbuf_width,
                             &self->priv->pixbuf_height);
        self->priv->width = self->priv->pixbuf_width * 1.0;
        self->priv->height = self->priv->pixbuf_height * 1.0;
        if (self->priv->pixbuf) {
          g_object_unref (self->priv->pixbuf);
        }
        self->priv->pixbuf = gdk_pixbuf_get_from_drawable (NULL,
                                                           GDK_DRAWABLE (pixmap),
                                                           gtk_widget_get_colormap (widget),
                                                           0, 0, 0, 0,
                                                           self->priv->pixbuf_width,
                                                           self->priv->pixbuf_height);
        g_object_unref (pixmap);
      }
    } break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
fade_out_func (gpointer data)
{
  GguFadeOutWidget *self = data;
  
  if (self->priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
    self->priv->width -= MAX (0.01, self->priv->pixbuf_width / self->priv->factor);
  } else {
    self->priv->height -= MAX (0.01, self->priv->pixbuf_height / self->priv->factor);
  }
  if ((gint) self->priv->width > 0 && (gint) self->priv->height > 0) {
    gtk_widget_queue_resize (GTK_WIDGET (self));
    
    return TRUE;
  } else {
    self->priv->source = NULL;
    gtk_widget_destroy (GTK_WIDGET (self));
    return FALSE;
  }
}

static gboolean
ggu_fade_out_widget_expose_event (GtkWidget      *widget,
                                  GdkEventExpose *event)
{
  GguFadeOutWidget *self    = GGU_FADE_OUT_WIDGET (widget);
  GdkWindow        *window  = gtk_widget_get_window (widget);
  gint              win_width;
  gint              win_height;
  cairo_t          *cr;
  
  gdk_window_get_size (window, &win_width, &win_height);
  
  cr = gdk_cairo_create (GDK_DRAWABLE (window));
  cairo_scale (cr,
               (gdouble) win_width / (gdouble) self->priv->pixbuf_width,
               (gdouble) win_height / (gdouble) self->priv->pixbuf_height);
  gdk_cairo_set_source_pixbuf (cr, self->priv->pixbuf, 0.0, 0.0);
  cairo_paint (cr);
  cairo_destroy (cr);
  
  return FALSE;
}

static void
ggu_fade_out_widget_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition)
{
  GguFadeOutWidget *self = GGU_FADE_OUT_WIDGET (widget);
  
  if (self->priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
    requisition->width = (gint) self->priv->width;
    requisition->height = 0;
  } else {
    requisition->width = 0;
    requisition->height = (gint) self->priv->height;
  }
}

static void
ggu_fade_out_widget_show (GtkWidget *widget)
{
  GguFadeOutWidget *self = GGU_FADE_OUT_WIDGET (widget);
  
  /* if we show the widget, start animation */
  if (! gtk_widget_get_visible (widget)) {
    guint duration;
    
    gtk_widget_style_get (widget, "anim-duration", &duration, NULL);
    
    self->priv->factor = MAX (2.0, (duration * 1.0) / (FRAME_DURATION * 1.0));
    self->priv->source = g_timeout_source_new (FRAME_DURATION);
    g_source_set_callback (self->priv->source, fade_out_func, self, NULL);
    g_source_attach (self->priv->source, NULL);
  }
  GTK_WIDGET_CLASS (ggu_fade_out_widget_parent_class)->show (widget);
}


/**
 * ggu_fade_out_widget_new:
 * @orientation: The animation orientation
 * @original: The widget to animate
 * 
 * Creates a new #GguFadeOutWidget for a given #GtkWidget.
 * 
 * Returns: a new #GguFadeOutWidget
 */
GtkWidget *
ggu_fade_out_widget_new (GtkOrientation orientation,
                         GtkWidget     *original)
{
  return g_object_new (GGU_TYPE_FADE_OUT_WIDGET,
                       "orientation", orientation,
                       "widget", original,
                       NULL);
}
