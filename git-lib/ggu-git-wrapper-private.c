
#include "ggu-git-wrapper-private.h"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>


typedef struct _GguGitWrapperPrivate GguGitWrapperPrivate;
struct _GguGitWrapperPrivate
{
  gint                  ref_count;
  
  gint                  stdout;
  GString              *stdout_str;
  gint                  stderr;
  GString              *stderr_str;
  guint                 reader_id;
  
  GguGitWrapperCallback callback;
  gpointer              callback_data;
};

GQuark
ggu_git_wrapper_error_quark (void)
{
  static GQuark q = 0;
  
  if (G_UNLIKELY (q == 0)) {
    q = g_quark_from_static_string ("ggu-git-wrapper");
  }
  
  return q;
}

static GguGitWrapperPrivate *
ggu_git_wrapper_private_new (void)
{
  GguGitWrapperPrivate *priv;
  
  priv = g_slice_alloc (sizeof *priv);
  priv->ref_count = 1;
  priv->stdout = -1;
  priv->stdout_str = g_string_new (NULL);
  priv->stderr = -1;
  priv->stderr_str = g_string_new (NULL);
  priv->reader_id = 0;
  
  return priv;
}

static GguGitWrapperPrivate *
ggu_git_wrapper_private_ref (GguGitWrapperPrivate *priv)
{
  g_atomic_int_inc (&priv->ref_count);
  return priv;
}

static void
ggu_git_wrapper_private_unref (GguGitWrapperPrivate *priv)
{
  if (g_atomic_int_dec_and_test (&priv->ref_count)) {
    g_string_free (priv->stdout_str, TRUE);
    g_string_free (priv->stderr_str, TRUE);
    g_slice_free1 (sizeof *priv, priv);
  }
}

static gboolean
fill_string_from_fd (GString *string,
                     gint     fd)
{
  gchar   buf[BUFSIZ];
  gssize  n_read;
  
  while ((n_read = read (fd, buf, sizeof (buf))) > 0) {
    g_string_append_len (string, buf, n_read);
  }
  
  return n_read >= 0;
}

static void
ggu_git_wrapper_fill_fds (GguGitWrapperPrivate *priv)
{
  fd_set          rfds;
  /* FIXME: is waiting 0ms a portable thing not for blocking? */
  struct timeval  tv = {0, 0};
  gint            rv;
  
  FD_ZERO (&rfds);
  FD_SET (priv->stdout, &rfds);
  FD_SET (priv->stderr, &rfds);
  rv = select (MAX (priv->stderr, priv->stdout) + 1, &rfds, NULL, NULL, &tv);
  if (rv < 0 && errno != EINTR) {
    g_warning ("select() failed: %s", g_strerror (errno));
  } else {
    if (FD_ISSET (priv->stdout, &rfds)) {
      fill_string_from_fd (priv->stdout_str, priv->stdout);
    }
    if (FD_ISSET (priv->stderr, &rfds)) {
      fill_string_from_fd (priv->stderr_str, priv->stderr);
    }
  }
}

static gboolean
ggu_git_wrapper_read_buffers_hanlder (gpointer data)
{
  GguGitWrapperPrivate *priv = ggu_git_wrapper_private_ref (data);
  gboolean              keep;
  
  keep = priv->reader_id > 0;
  if (keep) {
    ggu_git_wrapper_fill_fds (priv);
  }
  ggu_git_wrapper_private_unref (priv);
  
  return keep;
}

static void
ggu_git_wrapper_child_watch_hanlder (GPid     pid,
                                     gint     status,
                                     gpointer data)
{
  GguGitWrapperPrivate *priv = data;
  gboolean              success;
  gchar                *output = NULL;
  gchar                *error = NULL;
  
  g_source_remove (priv->reader_id);
  priv->reader_id = 0;
  g_spawn_close_pid (pid);
  success = WIFEXITED (status);
  if (success) {
    ggu_git_wrapper_fill_fds (priv);
    output = priv->stdout_str->str;
    error = priv->stderr_str->str;
  }
  priv->callback (success, WEXITSTATUS (status), output, error,
                  priv->callback_data);
  ggu_git_wrapper_private_unref (priv);
}

gboolean
ggu_git_wrapper (const gchar           *git_dir,
                 const gchar           *command,
                 const gchar          **args,
                 GguGitWrapperCallback callback,
                 gpointer               data,
                 GError               **error)
{
  static gchar         *env[] = { NULL };
  gchar               **argv;
  gsize                 n_args;
  gsize                 i = 0;
  GguGitWrapperPrivate *priv;
  gboolean              success = FALSE;
  GPid                  pid;
  
  n_args = (args ? g_strv_length ((gchar **)args) : 0) + 2;
  argv = g_malloc (sizeof *argv * (n_args + 1));
  argv[i++] = g_strdup ("git");
  argv[i++] = g_strdup (command);
  for (; args && *args; args++) {
    argv[i++] = g_strdup (*args);
  }
  argv[i] = NULL;
  
  priv = ggu_git_wrapper_private_new ();
  priv->callback = callback;
  priv->callback_data = data;
  if (! g_spawn_async_with_pipes (git_dir, argv, env,
                                  G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,
                                  NULL, NULL, &pid, NULL,
                                  &priv->stdout, &priv->stderr, error)) {
    g_debug ("child spawn failed: %s", error ? (*error)->message : "???");
    ggu_git_wrapper_private_unref (priv);
  } else {
    /* we need to read the child's pipes from time to time for the buffers not
     * to be fulfilled, and then block */
    priv->reader_id = g_timeout_add (5, ggu_git_wrapper_read_buffers_hanlder, priv);
    g_child_watch_add (pid, ggu_git_wrapper_child_watch_hanlder, priv);
    success = TRUE;
  }
  
  g_strfreev (argv);
  
  return success;
}

