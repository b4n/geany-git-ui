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

#ifndef H_GGU_UTILS
#define H_GGU_UTILS

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


/* sets @p to @v, calling @free_func on @p before if not @null */
#define ggu_set_pointer_full(p, v, f)  \
  G_STMT_START {                       \
    gpointer tmp = (p);                \
    (p) = (v);                         \
    if (tmp) {                         \
      (f) (tmp);                       \
    }                                  \
  } G_STMT_END

#define ggu_set_pointer(p, v)         ggu_set_pointer_full ((p), (v), g_free)
#define ggu_set_object_pointer(p, v)  ggu_set_pointer_full ((p), (v), g_object_unref)
#define ggu_unset_pointer(p)          ggu_set_pointer ((p), NULL)
#define ggu_unset_object_pointer(p)   ggu_set_object_pointer ((p), NULL)

#define GGU_SPTR   ggu_set_pointer
#define GGU_SOPTR  ggu_set_object_pointer
#define GGU_USPTR  ggu_unset_pointer
#define GGU_USOPTR ggu_unset_object_pointer


G_END_DECLS

#endif /* guard */
