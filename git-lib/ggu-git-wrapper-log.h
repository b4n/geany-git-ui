
#include <glib.h>

#ifndef H_GGU_GIT_WRAPPER_LOG
#define H_GGU_GIT_WRAPPER_LOG

G_BEGIN_DECLS


typedef struct _GguGitCommit GguGitCommit;
struct _GguGitCommit
{
  gint      ref_count;
  
  /*guint64   hash;*/
  gchar    *hash;
  gchar    *date; /* TODO: use a date repsentation? */
  gchar    *author;
  gchar    *summary;
  gchar    *details;
};

GguGitCommit     *ggu_git_commit_new      (void);
GguGitCommit     *ggu_git_commit_ref      (GguGitCommit *commit);
void              ggu_git_commit_unref    (GguGitCommit *commit);

typedef void (*GguGitLogCallback) (GList/*<GguGitCommit>*/ *commits,
                                   const GError            *error,
                                   gpointer                 data);

void            ggu_git_log               (const gchar       *dir,
                                           const gchar       *ref,
                                           const gchar       *file,
                                           GguGitLogCallback  callback,
                                           gpointer           data);
GList          *ggu_git_log_sync          (const gchar   *dir,
                                           const gchar   *ref,
                                           const gchar   *file,
                                           GError       **error);


G_END_DECLS

#endif /* guard */
