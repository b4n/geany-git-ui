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

/* inspired from GeanyWrapLabel */

#include "ggu-wrap-label.h"

#include <glib.h>
#include <gtk/gtk.h>


struct _GguWrapLabelPrivate
{
  gint wrap_width;
};


static void     ggu_wrap_label_size_request           (GtkWidget       *widget,
                                                       GtkRequisition  *req);
static void     ggu_wrap_label_size_allocate          (GtkWidget     *widget,
                                                       GtkAllocation *alloc);
static void     ggu_wrap_label_notify_label_handler   (GObject     *object,
                                                       GParamSpec  *pspec,
                                                       gpointer     data);


G_DEFINE_TYPE (GguWrapLabel,
               ggu_wrap_label,
               GTK_TYPE_LABEL)


static void
ggu_wrap_label_class_init (GguWrapLabelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  widget_class->size_request = ggu_wrap_label_size_request;
  widget_class->size_allocate = ggu_wrap_label_size_allocate;
  
  g_type_class_add_private (klass, sizeof (GguWrapLabelPrivate));
}

static void
ggu_wrap_label_init (GguWrapLabel *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_WRAP_LABEL,
                                            GguWrapLabelPrivate);
  
  self->priv->wrap_width = 0;
  
  gtk_misc_set_alignment (GTK_MISC (self), 0.0, 0.5);
  g_signal_connect (self, "notify::label",
                    G_CALLBACK (ggu_wrap_label_notify_label_handler), NULL);
  pango_layout_set_wrap (gtk_label_get_layout (GTK_LABEL (self)),
                         PANGO_WRAP_WORD_CHAR);
}

/* Sets the point at which the text should wrap. */
static void
ggu_wrap_label_set_wrap_width (GguWrapLabel *self,
                               gint          width)
{
  if (width > 0) {
    /* We may need to reset the wrap width, so do this regardless of whether
     * or not we've changed the width. */
    pango_layout_set_width (gtk_label_get_layout (GTK_LABEL (self)),
                            width * PANGO_SCALE);

    if (self->priv->wrap_width != width) {
      self->priv->wrap_width = width;
      gtk_widget_queue_resize (GTK_WIDGET (self));
    }
  }
}

/* updates the wrap width when the label text changes */
static void
ggu_wrap_label_notify_label_handler (GObject     *object,
                                     GParamSpec  *pspec,
                                     gpointer     data)
{
  GguWrapLabel *self = GGU_WRAP_LABEL (object);

  ggu_wrap_label_set_wrap_width (self, self->priv->wrap_width);
}

/* Forces the height to be the size necessary for the Pango layout, while allowing the
 * width to be flexible. */
static void
ggu_wrap_label_size_request (GtkWidget       *widget,
                             GtkRequisition  *req)
{
  req->width = 0;
  pango_layout_get_pixel_size (gtk_label_get_layout (GTK_LABEL (widget)),
                               NULL, &req->height);
}

/* Sets the wrap width to the width allocated to us. */
static void
ggu_wrap_label_size_allocate (GtkWidget      *widget,
                              GtkAllocation  *allocation)
{
  GTK_WIDGET_CLASS (ggu_wrap_label_parent_class)->size_allocate (widget,
                                                                 allocation);
  
  ggu_wrap_label_set_wrap_width (GGU_WRAP_LABEL (widget), allocation->width);
}


GtkWidget *
ggu_wrap_label_new (const gchar *label)
{
  return g_object_new (GGU_TYPE_WRAP_LABEL, "label", label, NULL);
}
