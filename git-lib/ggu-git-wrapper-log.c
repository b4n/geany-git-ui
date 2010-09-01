
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


/* checks if the litteral @h_l is at the start of @buf, and if so, moves the
 * start of @buf after that header */
#define HEADER_CHECK_AND_SKIP(buf, h_l)             \
  ((strncmp ((buf), (h_l), sizeof (h_l) - 1) == 0)  \
    ? (((buf) += sizeof (h_l) - 1), TRUE)           \
    : FALSE)

/* FIXME: this is ugly... */
static gboolean
ggu_git_commit_parse_from_log (GguGitCommit  *commit,
                               const gchar   *log,
                               const gchar  **entry_end,
                               GError       **error)
{
  const gchar  *endptr;
  gsize         len;
  GString      *sb;
  
  #define FAIL(msg) G_STMT_START {                                             \
    g_set_error (error, GGU_GIT_WRAPPER_ERROR,                                 \
                 GGU_GIT_WRAPPER_ERROR_INVALID_RESULT,                         \
                 "%s (data is \"%.16s...\")", msg, log);                       \
    goto failure;                                                              \
  } G_STMT_END
  
  if (! HEADER_CHECK_AND_SKIP (log, "commit ")) {
    FAIL ("Failed to read commit header");
  }
  #if 0
  commit->hash = g_ascii_strtoull (log, &endptr, 16);
  #else
  commit->hash = g_strndup (log, 40);
  endptr = log + 40;
  #endif
  if (endptr == log || *endptr != '\n') {
    FAIL ("Failed to read commit hash");
  }
  log = endptr + 1;
  
  /* skip merges */
  if (HEADER_CHECK_AND_SKIP (log, "Merge: ")) {
    while (*log && *log++ != '\n');
  }
  
  if (! HEADER_CHECK_AND_SKIP (log, "Author: ")) {
    FAIL ("Failed to read author header");
  }
  for (len = 0; log[len] && log[len] != '\n'; len ++);
  if (len < 1) {
    FAIL ("Failed to read author name");
  }
  commit->author = g_strndup (log, len);
  log += len + 1;
  
  if (! HEADER_CHECK_AND_SKIP (log, "Date:   ")) {
    FAIL ("Failed to read date header");
  }
  for (len = 0; log[len] && log[len] != '\n'; len ++);
  if (len < 29 || len > 30 || log[len] != '\n') {
    FAIL (("Failed to read date"));
  }
  commit->date = g_strndup (log, len);
  log += len + 1;
  
  if (*log != '\n') {
    FAIL ("Missing separator before commit message");
  }
  log ++;
  
  sb = g_string_new (NULL);
  while (*log && *log != '\n') {
    while (*log == ' ') log++;
    while (*log && *log != '\n') {
      g_string_append_c (sb, *log);
      log ++;
    }
    if (*log) {
      g_string_append_c (sb, *log);
      log ++;
    }
  }
  if (sb->len > 0 && sb->str[sb->len - 1] == '\n') {
    sb->str[--sb->len] = 0;
  }
  commit->details = g_string_free (sb, FALSE);
  
  for (len = 0; commit->details[len] && commit->details[len] != '\n'; len++);
  commit->summary = g_strndup (commit->details, len);
  
  /* skip post-message separator */
  if (*log == '\n') {
    log ++;
  }
  
  *entry_end = log;
  return TRUE;
  
 failure:
  g_free (commit->hash); commit->hash = NULL;
  g_free (commit->date); commit->date = NULL;
  g_free (commit->author); commit->author = NULL;
  g_free (commit->summary); commit->summary = NULL;
  g_free (commit->details); commit->details = NULL;
  return FALSE;
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
    const gchar *log = standard_output;
    
    while (*log) {
      GguGitCommit *commit;
      
      commit = ggu_git_commit_new ();
      if (! ggu_git_commit_parse_from_log (commit, log, &log, &error)) {
        ggu_git_commit_unref (commit);
        break;
      } else {
        commits = g_list_prepend (commits, commit);
      }
    }
    if (error) {
      ggu_git_commit_unref_list (commits);
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
  static const gchar *args[4]; /* ref, --, file, null */
  GError             *err = NULL;
  gsize               i = 0;
  GguGitLogPrivate   *priv;
  
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

