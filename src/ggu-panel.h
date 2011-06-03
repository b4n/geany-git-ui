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

#ifndef H_GGU_PANEL
#define H_GGU_PANEL

#include <glib.h>
#include <gtk/gtk.h>

#include "geanyplugin.h"
#include "document.h"

G_BEGIN_DECLS


#define GGU_TYPE_PANEL            (ggu_panel_get_type())
#define GGU_PANEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGU_TYPE_PANEL, GguPanel))
#define GGU_PANEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GGU_TYPE_PANEL, GguPanelClass))
#define GGU_IS_PANEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGU_TYPE_PANEL))
#define GGU_IS_PANEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GGU_TYPE_PANEL))
#define GGU_PANEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GGU_TYPE_PANEL, GguPanelClass))


typedef struct _GguPanel        GguPanel;
typedef struct _GguPanelClass   GguPanelClass;
typedef struct _GguPanelPrivate GguPanelPrivate;

struct _GguPanel
{
  GtkVBox parent_instance;
  GguPanelPrivate *priv;
};

struct _GguPanelClass
{
  GtkVBoxClass parent_class;
};


GType         ggu_panel_get_type                      (void) G_GNUC_CONST;
GtkWidget    *ggu_panel_new                           (void);
void          ggu_panel_set_document                  (GguPanel      *self,
                                                       GeanyDocument *doc);


G_END_DECLS

#endif /* guard */
