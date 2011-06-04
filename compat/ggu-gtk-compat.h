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

#ifndef H_GGU_GTK_COMPAT
#define H_GGU_GTK_COMPAT

#include <gtk/gtk.h>

G_BEGIN_DECLS


/* gdk_pixmap_get_size() */
#if ! defined (gdk_pixmap_get_size) && \
    ! GTK_CHECK_VERSION (2, 23, 4)
# define gdk_pixmap_get_size(p, w, h) \
  (gdk_drawable_get_size (GDK_DRAWABLE (p), (w), (h)))
#endif


G_END_DECLS

#endif /* guard */
