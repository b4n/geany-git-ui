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

#ifndef H_GGU_GLIB_COMPAT
#define H_GGU_GLIB_COMPAT

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


/* g_list_free_full() */
#if ! defined (g_list_free_full) && \
    ! GLIB_CHECK_VERSION (2, 27, 3)
static inline void
__GGU_g_list_free_full (GList          *list,
                        GDestroyNotify  destroy)
{
  g_list_foreach (list, (GFunc) destroy, NULL);
  g_list_free (list);
}
# define g_list_free_full __GGU_g_list_free_full
#endif

/* g_object_notify_by_pspec() */
#if ! defined (g_object_notify_by_pspec) && \
    ! GLIB_CHECK_VERSION (2, 25, 10)
static inline void
__GGU_g_object_notify_by_pspec (GObject    *object,
                                GParamSpec *pspec)
{
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (G_IS_PARAM_SPEC (pspec));
  
  g_object_notify (object, pspec->name);
}
# define g_object_notify_by_pspec __GGU_g_object_notify_by_pspec
#endif

/* g_simple_async_result_take_error() */
#if ! defined (g_simple_async_result_take_error) && \
    ! GLIB_CHECK_VERSION (2, 27, 3)
static inline void
__GGU_g_simple_async_result_take_error (GSimpleAsyncResult *simple,
                                        GError             *error)
{
  g_return_if_fail (G_IS_SIMPLE_ASYNC_RESULT (simple));
  g_return_if_fail (error != NULL);
  
  g_simple_async_result_set_from_error (simple, error);
  g_error_free (error);
}
# define g_simple_async_result_take_error __GGU_g_simple_async_result_take_error
#endif

/* G_DEFINE_BOXED_TYPE() -- stolen from GLib with slight modifications */
#ifndef G_DEFINE_BOXED_TYPE
# define G_DEFINE_BOXED_TYPE(TypeName, type_name, copy_func, free_func)        \
GType                                                                          \
type_name##_get_type (void)                                                    \
{                                                                              \
  static volatile gsize g_define_type_id__volatile = 0;                        \
  if (g_once_init_enter (&g_define_type_id__volatile))                         \
    {                                                                          \
      GType g_define_type_id =                                                 \
        g_boxed_type_register_static (g_intern_static_string (#TypeName),      \
                                      (GBoxedCopyFunc) copy_func,              \
                                      (GBoxedFreeFunc) free_func);             \
                                                                               \
      g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);       \
    }                                                                          \
  return g_define_type_id__volatile;                                           \
}
#endif


G_END_DECLS

#endif /* guard */
