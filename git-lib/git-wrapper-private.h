
#include <glib.h>

#ifndef H_GIT_WRAPPER_PRIVATE
#define H_GIT_WRAPPER_PRIVATE

G_BEGIN_DECLS


typedef void (*GitWrapperCallback)  (gboolean     success,
                                     gint         return_value,
                                     const gchar *standard_output,
                                     const gchar *standard_error,
                                     gpointer     data);

gboolean        git_wrapper         (const gchar         *git_dir,
                                     const gchar         *command,
                                     const gchar        **args,
                                     GitWrapperCallback   callback,
                                     gpointer             data,
                                     GError             **error);


/* mouahahah hack */
#ifndef g_list_free_full
#define g_list_free_full g_list_free_full
static void
g_list_free_full (GList      *l,
                  GFreeFunc   f)
{
  while (l) {
    GList *next = l->next;
    
    f (l->data);
    g_list_free_1 (l);
    l = next;
  }
}
#endif


G_END_DECLS

#endif /* guard */
