
#include <glib.h>

#ifndef H_GIT_WRAPPER_LOG
#define H_GIT_WRAPPER_LOG

G_BEGIN_DECLS


typedef struct _GitCommit GitCommit;
struct _GitCommit
{
  gint      ref_count;
  
  /*guint64   hash;*/
  gchar    *hash;
  gchar    *date; /* TODO: use a date repsentation? */
  gchar    *author;
  gchar    *summary;
  gchar    *details;
};

GitCommit      *git_commit_new      (void);
GitCommit      *git_commit_ref      (GitCommit *commit);
void            git_commit_unref    (GitCommit *commit);

typedef void (*GitLogCallback) (GList/*<GitCommit>*/ *commits,
                                const gchar          *error,
                                gpointer              data);

void            git_log             (const gchar   *dir,
                                     const gchar   *ref,
                                     const gchar   *file,
                                     GitLogCallback callback,
                                     gpointer       data);
GList          *git_log_sync        (const gchar   *dir,
                                     const gchar   *ref,
                                     const gchar   *file,
                                     GError       **error);


G_END_DECLS

#endif /* guard */
