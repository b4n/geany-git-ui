
#include <glib.h>

#ifndef H_GGU_GIT_WRAPPER_PRIVATE
#define H_GGU_GIT_WRAPPER_PRIVATE

G_BEGIN_DECLS


#define GGU_GIT_WRAPPER_ERROR (ggu_git_wrapper_error_quark ())

typedef enum _GguGitWrapperError
{
  GGU_GIT_WRAPPER_ERROR_CHILD_CRASHED,
  GGU_GIT_WRAPPER_ERROR_FAILED
} GguGitWrapperError;


typedef void (*GguGitWrapperCallback) (gboolean     success,
                                       gint         return_value,
                                       const gchar *standard_output,
                                       const gchar *standard_error,
                                       gpointer     data);

GQuark          ggu_git_wrapper_error_quark (void) G_GNUC_CONST;
gboolean        ggu_git_wrapper             (const gchar           *git_dir,
                                             const gchar           *command,
                                             const gchar          **args,
                                             GguGitWrapperCallback  callback,
                                             gpointer               data,
                                             GError               **error);


G_END_DECLS

#endif /* guard */
