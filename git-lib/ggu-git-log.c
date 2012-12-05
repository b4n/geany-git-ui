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

#include "ggu-git-log.h"

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-glib-compat.h"
#include "ggu-git.h"
#include "ggu-git-utils.h"
#include "ggu-git-log-entry.h"


GQuark
ggu_git_log_error_quark (void)
{
  static GQuark q = 0;
  
  if (G_UNLIKELY (q == 0)) {
    q = g_quark_from_static_string ("GguGitLog");
  }
  
  return q;
}


struct _GguGitLogPrivate
{
  gchar *rev;
  gchar *file;
};

enum
{
  PROP_0,
  
  PROP_REV,
  PROP_FILE
};


static void         ggu_git_log_get_property        (GObject    *object,
                                                     guint       prop_id,
                                                     GValue     *value,
                                                     GParamSpec *pspec);
static void         ggu_git_log_set_property        (GObject       *object,
                                                     guint          prop_id,
                                                     const GValue  *value,
                                                     GParamSpec    *pspec);
static void         ggu_git_log_finalize            (GObject *object);


G_DEFINE_TYPE (GguGitLog,
               ggu_git_log,
               GGU_TYPE_GIT)


static void
ggu_git_log_class_init (GguGitLogClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  
  object_class->get_property  = ggu_git_log_get_property;
  object_class->set_property  = ggu_git_log_set_property;
  object_class->finalize      = ggu_git_log_finalize;
  
  g_object_class_install_property (object_class, PROP_REV,
                                   g_param_spec_string ("rev",
                                                        "Rev",
                                                        "The revision to log",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_FILE,
                                   g_param_spec_string ("file",
                                                        "File",
                                                        "The file for which get the history",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguGitLogPrivate));
}

static void
ggu_git_log_init (GguGitLog *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GGU_TYPE_GIT_LOG,
                                            GguGitLogPrivate);
  
  self->priv->rev = NULL;
  self->priv->file = NULL;
}

static void
ggu_git_log_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GguGitLog *self = GGU_GIT_LOG (object);
  
  switch (prop_id) {
    case PROP_REV:
      g_value_set_string (value, self->priv->rev);
      break;
      
    case PROP_FILE:
      g_value_set_string (value, self->priv->file);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_log_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GguGitLog *self = GGU_GIT_LOG (object);
  
  switch (prop_id) {
    case PROP_REV: {
      const gchar *str = g_value_get_string (value);
      
      if (g_strcmp0 (self->priv->rev, str) != 0) {
        g_free (self->priv->rev);
        self->priv->rev = g_strdup (str);
        g_object_notify_by_pspec (object, pspec);
      }
    } break;
    
    case PROP_FILE: {
      const gchar *str = g_value_get_string (value);
      
      if (g_strcmp0 (self->priv->file, str) != 0) {
        g_free (self->priv->file);
        self->priv->file = g_strdup (str);
        g_object_notify_by_pspec (object, pspec);
      }
    } break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_log_finalize (GObject *object)
{
  GguGitLog *self = GGU_GIT_LOG (object);
  
  g_free (self->priv->rev);
  self->priv->rev = NULL;
  g_free (self->priv->file);
  self->priv->file = NULL;
  
  G_OBJECT_CLASS (ggu_git_log_parent_class)->finalize (object);
}



GguGitLog *
ggu_git_log_new (void)
{
  return g_object_new (GGU_TYPE_GIT_LOG, NULL);
}

/*
 * is_git_hash:
 * @hash: a string
 * 
 * Checks if a string is a possibly valid Git hash
 * 
 * Returns: whether @str looks OK
 */
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

/*
 * parse_message:
 * @msg: a raw commit message
 * 
 * Reformats @msg.
 * 
 * Returns: The reformatted message.
 */
static gchar *
parse_message (const gchar *msg)
{
  GString  *builder;
  gboolean  prev_newline = FALSE;
  gchar    *formatted;
  
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
  
  formatted = g_string_free (builder, FALSE);
  return g_strchomp (formatted);
}

static void
entry_list_unref (GList *entries)
{
  g_list_free_full (entries, (GDestroyNotify) ggu_git_log_entry_unref);
}

static void
ggu_git_log_parse_output (GguGit             *obj,
                          const gchar        *output,
                          GSimpleAsyncResult *result,
                          GCancellable       *cancellable)
{
  gchar **chunks;
  gsize   i = 0;
  GList  *entries = NULL;
  
  chunks = g_strsplit (output, "\xff", -1);
  for (i = 0; chunks[i] && (chunks[i][0] != '\n' || chunks[i][1]); i += 5) {
    GError         *error = NULL;
    GguGitLogEntry *entry;
    
    if (g_cancellable_set_error_if_cancelled (cancellable, &error)) {
      g_simple_async_result_take_error (result, error);
      break;
    }
    
    if (! chunks[i+1] || ! chunks[i+2] || ! chunks[i+3] || ! chunks[i+4]) {
      g_simple_async_result_set_error (result, GGU_GIT_LOG_ERROR,
                                       GGU_GIT_LOG_ERROR_INCOMPLETE_RESULT,
                                       "Incomplete output");
      break;
    }
    /* there is a leading \n after each entry, then before each hash that is
     * not the first one */
    g_strchug (chunks[i]);
    if (! is_git_hash (chunks[i])) {
      g_simple_async_result_set_error (result, GGU_GIT_LOG_ERROR,
                                       GGU_GIT_LOG_ERROR_INVALID_RESULT,
                                       "Corrupted output: don't start with a hash");
      break;
    }
    
    entry = ggu_git_log_entry_new ();
    entry->hash    = g_strdup (chunks[i+0]);
    entry->date    = ggu_git_utf8_ensure_valid (chunks[i+1]);
    entry->author  = ggu_git_utf8_ensure_valid (chunks[i+2]);
    entry->summary = ggu_git_utf8_ensure_valid (chunks[i+3]);
    entry->details = parse_message (chunks[i+4]);
    
    entries = g_list_prepend (entries, entry);
  }
  entries = g_list_reverse (entries);
  g_simple_async_result_set_op_res_gpointer (result, entries,
                                             (GDestroyNotify) entry_list_unref);
  g_strfreev (chunks);
}

static gchar **
ggu_git_log_get_argv (GguGitLog *self)
{
  GPtrArray *argv;
  
  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, g_strdup ("git"));
  g_ptr_array_add (argv, g_strdup ("log"));
  g_ptr_array_add (argv, g_strdup ("--format="
                                   "%H%xff"
                                   "%aD%xff"
                                   "%an <%ae>%xff"
                                   "%s%xff"
                                   "%B%xff"));
  /* support to log on no real branch, in which case we long on current state */
  if (self->priv->rev && strcmp (self->priv->rev, "(no branch)") != 0) {
    g_ptr_array_add (argv, g_strdup (self->priv->rev));
  }
  if (self->priv->file) {
    g_ptr_array_add (argv, g_strdup ("--"));
    g_ptr_array_add (argv, g_strdup (self->priv->file));
  }
  g_ptr_array_add (argv, NULL);
  
  return (gchar **) g_ptr_array_free (argv, FALSE);
}

void
ggu_git_log_log_async (GguGitLog           *self,
                       const gchar         *dir,
                       const gchar         *rev,
                       const gchar         *file,
                       GCancellable        *cancellable,
                       GAsyncReadyCallback  callback,
                       gpointer             user_data)
{
  gchar **argv;
  
  g_object_set (self,
                "dir", dir,
                "rev", rev,
                "file", file,
                NULL);
  
  argv = ggu_git_log_get_argv (self);
  _ggu_git_run_async (GGU_GIT (self), argv, ggu_git_log_parse_output,
                      G_PRIORITY_DEFAULT, cancellable, callback, user_data);
  g_strfreev (argv);
}

/**
 * ggu_git_log_log_finish:
 * @self: A #GguGitLog
 * @result: The #GAsyncResult
 * @error: return location for errors or %NULL to ignore
 * 
 * Fetches the result of the operation started with ggu_git_log_log_async().
 * 
 * Returns: (transfer none): A list of GguGitLogEntry
 */
GList *
ggu_git_log_log_finish (GguGitLog    *self,
                        GAsyncResult *result,
                        GError      **error)
{
  return _ggu_git_run_finish (GGU_GIT (self), result, error);
}
