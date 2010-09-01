
#include "ggu-git-wrapper-private.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h> /* For BUFSIZ */
#include <glib.h>


typedef struct _IoSource IoSource;
struct _IoSource
{
  GIOChannel   *channel;
  guint         source_id;
  GString      *str;
};

typedef struct _GguGitWrapperPrivate GguGitWrapperPrivate;
struct _GguGitWrapperPrivate
{
  gint                  ref_count;
  
  IoSource              stdout;
  IoSource              stderr;
  
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
  priv->stdout.channel = NULL;
  priv->stdout.source_id = 0;
  priv->stdout.str = g_string_new (NULL);
  priv->stderr.channel = NULL;
  priv->stderr.source_id = 0;
  priv->stderr.str = g_string_new (NULL);
  
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
    if (priv->stdout.source_id) g_source_remove (priv->stdout.source_id);
    g_string_free (priv->stdout.str, TRUE);
    if (priv->stderr.source_id) g_source_remove (priv->stderr.source_id);
    g_string_free (priv->stderr.str, TRUE);
    g_slice_free1 (sizeof *priv, priv);
  }
}

static gboolean
ggu_git_wrapper_io_watch_handler (GIOChannel   *channel,
                                  GIOCondition  cond,
                                  gpointer      data)
{
  IoSource *io = data;
  
  switch (cond) {
    case G_IO_IN:
    case G_IO_PRI: {
      gsize n_read;
      
      do {
        gchar   buf[BUFSIZ];
        GError *err = NULL;
        
        n_read = 0;
        switch (g_io_channel_read_chars (channel, buf, sizeof (buf), &n_read,
                                         &err)) {
          case G_IO_STATUS_ERROR:
            g_warning ("Read failed: %s", err->message);
            /* Fallthrough */
          case G_IO_STATUS_EOF:
            io->channel = NULL;
            io->source_id = 0;
            break;
          
          case G_IO_STATUS_NORMAL:
            g_string_append_len (io->str, buf, (gssize)n_read);
            break;
          
          case G_IO_STATUS_AGAIN:
            continue;
        }
      } while (n_read > 0);
      break;
    }
    
    case G_IO_ERR:
      g_warning ("IO channel error");
      break;
    
    default:;
  }
  
  return io->channel != NULL;
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
  
  g_spawn_close_pid (pid);
  success = WIFEXITED (status);
  if (success) {
    output = priv->stdout.str->str;
    error = priv->stderr.str->str;
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
  gint                  stdout_fd;
  gint                  stderr_fd;
  
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
                                  &stdout_fd, &stderr_fd, error)) {
    g_debug ("child spawn failed: %s", error ? (*error)->message : "???");
    ggu_git_wrapper_private_unref (priv);
  } else {
    /* we need to read the child's pipes from time to time for the buffers not
     * to be fulfilled, and then block */
    #define ADD_WATCH(io, fd) \
      ((io)->channel = g_io_channel_unix_new ((fd)),                           \
       g_io_channel_set_encoding ((io)->channel, NULL, NULL),                  \
       (io)->source_id = g_io_add_watch ((io)->channel, G_IO_IN | G_IO_PRI,    \
                                         ggu_git_wrapper_io_watch_handler, io),\
       g_io_channel_unref ((io)->channel),                                     \
       (io)->source_id)
    
    ADD_WATCH (&priv->stdout, stdout_fd);
    ADD_WATCH (&priv->stderr, stderr_fd);
    g_child_watch_add (pid, ggu_git_wrapper_child_watch_hanlder, priv);
    success = TRUE;
  }
  
  g_strfreev (argv);
  
  return success;
}

