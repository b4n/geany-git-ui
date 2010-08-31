
#include <glib.h>

#ifndef H_GIT_WRAPPER_PRIVATE
#define H_GIT_WRAPPER_PRIVATE

G_BEGIN_DECLS


#define GIT_WRAPPER_ERROR (git_wrapper_error_quark ())

typedef enum _GitWrapperError
{
  GIT_WRAPPER_ERROR_CHILD_CRASHED,
  GIT_WRAPPER_ERROR_FAILED
} GitWrapperError;


typedef void (*GitWrapperCallback)  (gboolean     success,
                                     gint         return_value,
                                     const gchar *standard_output,
                                     const gchar *standard_error,
                                     gpointer     data);

GQuark          git_wrapper_error_quark (void) G_GNUC_CONST;
gboolean        git_wrapper             (const gchar         *git_dir,
                                         const gchar         *command,
                                         const gchar        **args,
                                         GitWrapperCallback   callback,
                                         gpointer             data,
                                         GError             **error);


G_END_DECLS

#endif /* guard */
