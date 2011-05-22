
#include "ggu-git-wrapper-log.h"

#include <glib.h>
#include <string.h>

#include "ggu-git-utils.h"
#include "ggu-git-wrapper-private.h"


GguGitCommit *
ggu_git_commit_new (void)
{
  GguGitCommit *commit;
  
  commit = g_slice_alloc0 (sizeof *commit);
  commit->ref_count = 1;
  
  return commit;
}

GguGitCommit *
ggu_git_commit_ref (GguGitCommit *commit)
{
  g_atomic_int_inc (&commit->ref_count);
  return commit;
}

void
ggu_git_commit_unref (GguGitCommit *commit)
{
  if (g_atomic_int_dec_and_test (&commit->ref_count)) {
    g_free (commit->hash);
    g_free (commit->date);
    g_free (commit->author);
    g_free (commit->summary);
    g_free (commit->details);
    g_slice_free1 (sizeof *commit, commit);
  }
}

static void
ggu_git_commit_unref_list (GList *commits)
{
  #if 1
  ggu_git_list_free_full (commits, (GFreeFunc)ggu_git_commit_unref);
  #else
  while (commits) {
    GList *next = commits->next;
    
    git_commit_unref (commits->data);
    g_list_free_1 (commits);
    commits = next;
  }
  #endif
}

static gboolean
is_git_hash (const gchar *hash)
{
  guint i;
  
  for (i = 0; hash[i]; i++) {
    if (! g_ascii_isxdigit (hash[i])) {
      return FALSE;
    }
  }
  
  return i == 40;
}

static gchar *
parse_message (const gchar *msg)
{
  GString  *builder;
  gboolean  prev_newline = FALSE;
  
  builder = g_string_new (NULL);
  for (; *msg; msg++) {
    if (! prev_newline && *msg == '\n' && g_ascii_isalnum (msg[1])) {
      /* transform this newline by a space */
      g_string_append_c (builder, ' ');
    } else {
      g_string_append_c (builder, *msg);
    }
    prev_newline = *msg == '\n';
  }
  
  return g_string_free (builder, FALSE);
}

static GList/*<GguGitCommit *>*/ *
parse_log (const gchar   *log,
           GError       **error)
{
  gchar **chunks;
  gsize   i = 0;
  GList  *commits = NULL;
  
  chunks = g_strsplit (log, "\xff", -1);
  for (i = 0; chunks[i] && (chunks[i][0] != '\n' || chunks[i][1]); i += 5) {
    GguGitCommit *commit;
    
    if (! chunks[i+1] || ! chunks[i+2] || ! chunks[i+3] || ! chunks[i+4]) {
      g_set_error (error, GGU_GIT_WRAPPER_ERROR,
                   GGU_GIT_WRAPPER_ERROR_INVALID_RESULT,
                   "Incomplete output");
      break;
    }
    /* there is a leading \n after each entry, then before each hash that is
     * not the first one */
    g_strchug (chunks[i]);
    if (! is_git_hash (chunks[i])) {
      g_set_error (error, GGU_GIT_WRAPPER_ERROR,
                   GGU_GIT_WRAPPER_ERROR_INVALID_RESULT,
                   "Corrupted output: don't start with a hash");
      break;
    }
    
    commit = ggu_git_commit_new ();
    commit->hash    = g_strdup (chunks[i+0]);
    commit->date    = g_strdup (chunks[i+1]);
    commit->author  = g_strdup (chunks[i+2]);
    commit->summary = g_strdup (chunks[i+3]);
    /* FIXME: parse chunks[i+4] to not have useless \n */
    commit->details = parse_message (chunks[i+4]);
    
    commits = g_list_prepend (commits, commit);
  }
  g_strfreev (chunks);
  
  return commits;
}


typedef struct _GguGitLogPrivate GguGitLogPrivate;
struct _GguGitLogPrivate
{
  GguGitLogCallback callback;
  gpointer          callback_data;
};

static void
ggu_git_log_finish_callback (gboolean     success,
                             gint         return_value,
                             const gchar *standard_output,
                             const gchar *standard_error,
                             gpointer     data)
{
  GguGitLogPrivate *priv = data;
  GList            *commits = NULL;
  GError           *error = NULL;
  
  if (! success) {
    error = g_error_new_literal (GGU_GIT_WRAPPER_ERROR,
                                 GGU_GIT_WRAPPER_ERROR_CHILD_CRASHED,
                                 "Git crashed");
  } else if (return_value != 0) {
    error = g_error_new (GGU_GIT_WRAPPER_ERROR, GGU_GIT_WRAPPER_ERROR_FAILED,
                         "Git failed: %s", standard_error);
  } else {
    commits = parse_log (standard_output, &error);
    if (error) {
      ggu_git_commit_unref_list (commits);
      commits = NULL;
    } else {
      /* reverse list order */
      commits = g_list_reverse (commits);
    }
  }
  
  priv->callback (commits, error, priv->callback_data);
  if (error) g_error_free (error);
  ggu_git_commit_unref_list (commits);
  g_slice_free1 (sizeof *priv, priv);
}

void
ggu_git_log (const gchar       *dir,
             const gchar       *ref,
             const gchar       *file,
             GguGitLogCallback  callback,
             gpointer           data)
{
  static const gchar *args[5]; /* --format=format, ref, --, file, null */
  GError             *err = NULL;
  gsize               i = 0;
  GguGitLogPrivate   *priv;
  
  args[i++] = "--format="
              "%H%xff"
              "%aD%xff"
              "%an <%ae>%xff"
              "%s%xff"
              "%B%xff";
  /* support to log on no real branch, in which case we long on current state */
  if (ref && strcmp (ref, "(no branch)") != 0) {
    args[i++] = ref;
  }
  if (file) {
    args[i++] = "--";
    args[i++] = file;
  }
  args[i] = NULL;
  
  priv = g_slice_alloc (sizeof *priv);
  priv->callback = callback;
  priv->callback_data = data;
  
  if (! ggu_git_wrapper (dir, "log", args,
                         ggu_git_log_finish_callback, priv, &err)) {
    /* FIXME: call this from inside the GMainLoop for the thread things to be
     * the same as a successful call.
     * Use E.g. a timeout (very short) or an idle */
    callback (NULL, err, data);
    g_error_free (err);
    g_slice_free1 (sizeof *priv, priv);
  }
}


/* fake synchronous version */
typedef struct _GguGitLogSyncPrivate GguGitLogSyncPrivate;
struct _GguGitLogSyncPrivate
{
  GList    *commits;
  GError   *error;
  gboolean  done;
};

static void
ggu_git_log_sync_result_callback (GList        *commits,
                                  const GError *error,
                                  gpointer      data)
{
  GguGitLogSyncPrivate *priv = data;
  
  if (error) {
    priv->error = g_error_copy (error);
  } else {
    for (; commits; commits = commits->next) {
      priv->commits = g_list_prepend (priv->commits,
                                      ggu_git_commit_ref (commits->data));
    }
    priv->commits = g_list_reverse (priv->commits);
  }
  priv->done = TRUE;
}

#include <stdio.h>

/* Hackish synchronous version of git_log()
 * Note that this is not a really syncronous version an that it only simulates
 * synchronicity by creating forcing main loop iterations until it gets the
 * result. This may lead to side effets, I don't know.
 * For example, a side effect is that even if for the direct caller it looks
 * like a synchronous version, some IDLE or timout callbacks may have run
 * meanwhile. */
GList/*<GguGitCommit>*/ *
ggu_git_log_sync (const gchar   *dir,
                  const gchar   *ref,
                  const gchar   *file,
                  GError       **error)
{
  GguGitLogSyncPrivate priv;
  
  priv.commits  = NULL;
  priv.error    = NULL;
  priv.done     = FALSE;
  ggu_git_log (dir, ref, file, ggu_git_log_sync_result_callback, &priv);
  while (! priv.done) {
    g_main_iteration (TRUE);
  }
  if (priv.error) {
    g_propagate_error (error, priv.error);
  }
  
  return priv.commits;
}

