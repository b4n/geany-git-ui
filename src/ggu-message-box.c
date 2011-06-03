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

/*
 * This is a widget that behaves like a GtkBox but that have a maximum number
 * of children. If more children are added, the older one gets removed to leave
 * some place.
 * 
 * This is intended to display messages that informs the user (infos, warnings,
 * errors, etc.), so she has the oppotunity to read them, but don't get flooded
 * by them.
 * 
 * FIXME:
 * Impelement soft limit
 * 
 * FIXME:
 * The implementation has a huge flaw: since it's not possible to be notified
 * of the addition of a child when it is added with gtk_box_pack_*(), one MUST
 * NOT use one of these functions to add a child to this widget, but only
 * gtk_container_add().
 * 
 * Other possibilities with their flaws:
 * 
 * * Making a complete GtkContainer subclass is a pain
 * * Making a GtkContainer subclass that uses a GtkBox as proxy looks not too
 *   hard, but gtk_container_remove() needs the child to remove to be a direct
 *   child of the container on which the action is performed.
 */

#include "ggu-message-box.h"

#include "config.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "ggu-fade-out-widget.h"


struct _GguMessageBoxPrivate
{
  GQueue         *children;
  
  guint           soft_limit;
  guint           hard_limit;
};


static void     ggu_message_box_finalize                      (GObject *object);
static void     ggu_message_box_get_propery                   (GObject    *object,
                                                               guint       prop_id,
                                                               GValue     *value,
                                                               GParamSpec *pspec);
static void     ggu_message_box_set_propery                   (GObject      *object,
                                                               guint         prop_id,
                                                               const GValue *value,
                                                               GParamSpec   *pspec);
static void     ggu_message_box_add                           (GtkContainer *container,
                                                               GtkWidget    *child);
static void     ggu_message_box_remove                        (GtkContainer *container,
                                                               GtkWidget    *child);
static void     ggu_message_box_release_child                 (GguMessageBox *self,
                                                               GtkWidget     *child);


G_DEFINE_TYPE (GguMessageBox,
               ggu_message_box,
               GTK_TYPE_BOX)


enum
{
  PROP_0,
  
  PROP_ORIENTATION,
  
  PROP_HARD_LIMIT,
  PROP_SOFT_LIMIT
};


static void
ggu_message_box_finalize (GObject *object)
{
  GguMessageBox *self = GGU_MESSAGE_BOX (object);
  
  g_queue_free (self->priv->children);
  self->priv->children = NULL;
  
  G_OBJECT_CLASS (ggu_message_box_parent_class)->finalize (object);
}

static void
ggu_message_box_class_init (GguMessageBoxClass *klass)
{
  GObjectClass       *object_class    = G_OBJECT_CLASS (klass);
  GtkContainerClass  *container_class = GTK_CONTAINER_CLASS (klass);
  
  object_class->finalize      = ggu_message_box_finalize;
  object_class->get_property  = ggu_message_box_get_propery;
  object_class->set_property  = ggu_message_box_set_propery;
  
  container_class->add    = ggu_message_box_add;
  container_class->remove = ggu_message_box_remove;
  
  g_object_class_install_property (object_class,
                                   PROP_HARD_LIMIT,
                                   g_param_spec_uint ("hard-limit",
                                                      "Hard limit",
                                                      "Maximum number of children",
                                                      0,
                                                      G_MAXUINT,
                                                      10,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class,
                                   PROP_SOFT_LIMIT,
                                   g_param_spec_uint ("soft-limit",
                                                      "Soft limit",
                                                      "Maximum number of children",
                                                      0,
                                                      G_MAXUINT,
                                                      3,
                                                      G_PARAM_READWRITE |
                                                      G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguMessageBoxPrivate));
}

static void
ggu_message_box_init (GguMessageBox *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GGU_TYPE_MESSAGE_BOX,
                                            GguMessageBoxPrivate);
  
  self->priv->children = g_queue_new ();
  self->priv->hard_limit = 10;
  self->priv->soft_limit = 3;
}

static void
ggu_message_box_get_propery (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GguMessageBox *self = GGU_MESSAGE_BOX (object);
  
  switch (prop_id) {
    case PROP_HARD_LIMIT:
      g_value_set_uint (value, self->priv->hard_limit);
      break;
      
    case PROP_SOFT_LIMIT:
      g_value_set_uint (value, self->priv->soft_limit);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_message_box_set_propery (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GguMessageBox *self = GGU_MESSAGE_BOX (object);
  
  switch (prop_id) {
    case PROP_HARD_LIMIT:
      ggu_message_box_set_hard_limit (self, g_value_get_uint (value));
      break;
      
    case PROP_SOFT_LIMIT:
      ggu_message_box_set_soft_limit (self, g_value_get_uint (value));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_message_box_apply_limits (GguMessageBox *self)
{
  guint n_children;
  
  n_children = g_queue_get_length (self->priv->children);
  while (n_children > self->priv->hard_limit) {
    GtkWidget *child;
    
    child = g_queue_peek_head (self->priv->children);
    ggu_message_box_remove (GTK_CONTAINER (self), child);
    n_children --;
  }
}

static void
ggu_message_box_add (GtkContainer *container,
                     GtkWidget    *child)
{
  GguMessageBox *self = GGU_MESSAGE_BOX (container);
  
  g_queue_push_tail (self->priv->children, child);
  g_signal_connect_swapped (child, "destroy",
                            G_CALLBACK (ggu_message_box_release_child), self);
  ggu_message_box_apply_limits (self);
  gtk_box_pack_end (GTK_BOX (self), child, TRUE, TRUE, 0);
}

static void
ggu_message_box_release_child (GguMessageBox *self,
                               GtkWidget     *child)
{
  g_queue_remove (self->priv->children, child);
}

static void
ggu_message_box_real_remove (GtkContainer *container,
                             GtkWidget    *child)
{
  ggu_message_box_release_child (GGU_MESSAGE_BOX (container), child);
  GTK_CONTAINER_CLASS (ggu_message_box_parent_class)->remove (container, child);
}

/* A bit hackish, finds the position of a child widget.
 * This supposes children are packed at the end. */
static gint
get_child_position (GtkContainer *container,
                    GtkWidget    *child)
{
  gint    i = -1;
  gint    n = 0;
  GList  *children;
  GList  *item;
  
  children = gtk_container_get_children (container);
  for (item = children; item; item = item->next) {
    if (item->data == child) {
      i = n;
    }
    n ++;
  }
  g_list_free (children);
  
  return n - i;
}

static void
ggu_message_box_remove (GtkContainer *container,
                        GtkWidget    *child)
{
  GguMessageBox *self = GGU_MESSAGE_BOX (container);
  
  if (g_queue_find (self->priv->children, child) &&
      gtk_widget_get_realized (child)) {
    GtkWidget *anim;
    gint       child_pos;
    
    anim = ggu_fade_out_widget_new (gtk_orientable_get_orientation (GTK_ORIENTABLE (self)),
                                    child);
    child_pos = get_child_position (container, child);
    gtk_box_pack_end (GTK_BOX (self), anim, TRUE, TRUE, 0);
    gtk_box_reorder_child (GTK_BOX (self), anim, child_pos);
    ggu_message_box_real_remove (container, child);
    gtk_widget_show (anim);
  } else {
    ggu_message_box_real_remove (container, child);
  }
}


GtkWidget *
ggu_message_box_new (GtkOrientation orientation)
{
  return g_object_new (GGU_TYPE_MESSAGE_BOX,
                       "orientation", orientation,
                       NULL);
}

void
ggu_message_box_set_hard_limit (GguMessageBox *self,
                                guint          hard_limit)
{
  g_return_if_fail (GGU_IS_MESSAGE_BOX (self));
  g_return_if_fail (hard_limit >= self->priv->soft_limit);
  
  if (self->priv->hard_limit != hard_limit) {
    self->priv->hard_limit = hard_limit;
    ggu_message_box_apply_limits (self);
    g_object_notify (G_OBJECT (self), "hard-limit");
  }
}

guint
ggu_message_box_get_hard_limit (GguMessageBox *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_BOX (self), 0);
  
  return self->priv->hard_limit;
}

void
ggu_message_box_set_soft_limit (GguMessageBox *self,
                                guint          soft_limit)
{
  g_return_if_fail (GGU_IS_MESSAGE_BOX (self));
  g_return_if_fail (soft_limit <= self->priv->hard_limit);
  
  if (self->priv->soft_limit != soft_limit) {
    self->priv->soft_limit = soft_limit;
    ggu_message_box_apply_limits (self);
    g_object_notify (G_OBJECT (self), "soft-limit");
  }
}

guint
ggu_message_box_get_soft_limit (GguMessageBox *self)
{
  g_return_val_if_fail (GGU_IS_MESSAGE_BOX (self), 0);
  
  return self->priv->soft_limit;
}
