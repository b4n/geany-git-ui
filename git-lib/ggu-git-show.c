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
 */

#include "ggu-git-show.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-glib-compat.h"
#include "ggu-git.h"
#include "ggu-git-utils.h"
#include "ggu-git-files-changed-entry.h"
#include "ggu-git-blame-entry.h"


struct _GguGitShowPrivate
{
  gchar    *rev;
  gchar    *file;
  gboolean  diff;
};

enum
{
  PROP_0,
  
  PROP_REV,
  PROP_FILE,
  PROP_DIFF
};


static void         ggu_git_show_get_property       (GObject    *object,
                                                     guint       prop_id,
                                                     GValue     *value,
                                                     GParamSpec *pspec);
static void         ggu_git_show_set_property       (GObject       *object,
                                                     guint          prop_id,
                                                     const GValue  *value,
                                                     GParamSpec    *pspec);
static void         ggu_git_show_finalize           (GObject *object);


G_DEFINE_TYPE (GguGitShow,
               ggu_git_show,
               GGU_TYPE_GIT)


static void
ggu_git_show_class_init (GguGitShowClass *klass)
{
  GObjectClass *object_class  = G_OBJECT_CLASS (klass);
  
  object_class->get_property  = ggu_git_show_get_property;
  object_class->set_property  = ggu_git_show_set_property;
  object_class->finalize      = ggu_git_show_finalize;
  
  g_object_class_install_property (object_class, PROP_REV,
                                   g_param_spec_string ("rev",
                                                        "Rev",
                                                        "The revision to show",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_FILE,
                                   g_param_spec_string ("file",
                                                        "File",
                                                        "The file to show",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_DIFF,
                                   g_param_spec_boolean ("diff",
                                                         "Diff",
                                                         "Whether to get the diff or the full content",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));
  
  g_type_class_add_private (klass, sizeof (GguGitShowPrivate));
}

static void
ggu_git_show_init (GguGitShow *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            GGU_TYPE_GIT_SHOW,
                                            GguGitShowPrivate);
  
  self->priv->rev = NULL;
  self->priv->file = NULL;
  self->priv->diff = FALSE;
}

static void
ggu_git_show_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  GguGitShow *self = GGU_GIT_SHOW (object);
  
  switch (prop_id) {
    case PROP_REV:
      g_value_set_string (value, self->priv->rev);
      break;
      
    case PROP_FILE:
      g_value_set_string (value, self->priv->file);
      break;
      
    case PROP_DIFF:
      g_value_set_boolean (value, self->priv->diff);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_show_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  GguGitShow *self = GGU_GIT_SHOW (object);
  
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
    
    case PROP_DIFF: {
      gboolean val = g_value_get_boolean (value);
      
      if (val != self->priv->diff) {
        self->priv->diff = val;
        g_object_notify_by_pspec (object, pspec);
      }
    } break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ggu_git_show_finalize (GObject *object)
{
  GguGitShow *self = GGU_GIT_SHOW (object);
  
  g_free (self->priv->rev);
  self->priv->rev = NULL;
  g_free (self->priv->file);
  self->priv->file = NULL;
  
  G_OBJECT_CLASS (ggu_git_show_parent_class)->finalize (object);
}



GguGitShow *
ggu_git_show_new (void)
{
  return g_object_new (GGU_TYPE_GIT_SHOW, NULL);
}


static void
ggu_git_show_parse_output (GguGit             *obj,
                           const gchar        *output,
                           GSimpleAsyncResult *result,
                           GCancellable       *cancellable)
{
  g_simple_async_result_set_op_res_gpointer (result, g_strdup (output), g_free);
}

static gchar **
ggu_git_show_get_argv (GguGitShow *self)
{
  GPtrArray *argv;
  
  argv = g_ptr_array_new ();
  g_ptr_array_add (argv, g_strdup ("git"));
  g_ptr_array_add (argv, g_strdup ("show"));
  if (! self->priv->diff && self->priv->file) {
    g_ptr_array_add (argv,
                     g_strdup_printf ("%s:%s",
                                      self->priv->rev ? self->priv->rev : "",
                                      self->priv->file));
  } else {
    if (self->priv->rev) {
      g_ptr_array_add (argv, g_strdup (self->priv->rev));
    }
    if (self->priv->file) {
      g_ptr_array_add (argv, g_strdup ("--"));
      g_ptr_array_add (argv, g_strdup (self->priv->file));
    }
  }
  g_ptr_array_add (argv, NULL);
  
  return (gchar **) g_ptr_array_free (argv, FALSE);
}

void
ggu_git_show_show_async (GguGitShow          *self,
                         const gchar         *dir,
                         const gchar         *rev,
                         const gchar         *file,
                         gboolean             diff,
                         GCancellable        *cancellable,
                         GAsyncReadyCallback  callback,
                         gpointer             user_data)
{
  gchar **argv;
  
  g_return_if_fail (diff || file); /* can't get content without file */
  
  g_object_set (self,
                "dir", dir,
                "rev", rev,
                "file", file,
                "diff", diff,
                NULL);
  
  argv = ggu_git_show_get_argv (self);
  _ggu_git_run_async (GGU_GIT (self), argv, ggu_git_show_parse_output,
                      G_PRIORITY_DEFAULT, cancellable, callback, user_data);
  g_strfreev (argv);
}

const gchar *
ggu_git_show_show_finish (GguGitShow   *self,
                          GAsyncResult *result,
                          GError      **error)
{
  return _ggu_git_run_finish (GGU_GIT (self), result, error);
}


/* list files changed */

static void
files_changed_entry_list_unref (GList *entries)
{
  g_list_free_full (entries,
                    (GDestroyNotify) ggu_git_files_changed_entry_unref);
}

static inline gboolean
unescape_filename (gchar              *filename,
                   GSimpleAsyncResult *result)
{
  if (*filename == '"') {
    guint i, j;
    
    for (i = 1, j = 0;
         filename[i] != 0 && (filename[i] != '"' || filename[i+1] != 0);
         i++) {
      if (filename[i] == '\\') {
        i++; /* skip backslash */
        switch (filename[i]) {
          case 'n':   filename[j++] = '\n'; break;
          case 't':   filename[j++] = '\t'; break;
          case '"':
          case '\\':  filename[j++] = filename[i]; break;
          
          default:
            /* should not happen */
            g_warning ("unexpected escaping");
            i--;
            filename[j++] = filename[i];
        }
      } else {
        filename[j++] = filename[i];
      }
    }
    if (filename[i] != '"') {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Unexpected end of line");
      return FALSE;
    }
    filename[j] = 0;
  }
  
  return TRUE;
}

static inline gboolean
parse_changes_count (const gchar        *changes,
                     guint              *n_,
                     GSimpleAsyncResult *result)
{
  if (changes[0] == '-' && changes[1] == 0) {
    *n_ = 0u;
  } else {
    gulong  n;
    gchar  *end;
    
    n = strtoul (changes, &end, 10);
    if (*end != 0) {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Invalid change count \"%s\"", changes);
      return FALSE;
    } else if (n > G_MAXUINT) {
      g_warning ("value too big, truncating");
    }
    *n_ = (guint) n;
  }
  
  return TRUE;
}

static void
ggu_git_list_files_changed_parse_output (GguGit             *obj,
                                         const gchar        *output,
                                         GSimpleAsyncResult *result,
                                         GCancellable       *cancellable)
{
  /* Formats:
   * 
   * n_added <\t> n_removed <\t> file-name
   * n_added <\t> n_removed <\t> "file-name-with-special-chars"
   * -       <\t> -         <\t> binary-file-name
   * -       <\t> -         <\t> "binary-file-name-with-special-chars"
   */
  /* FIXME: would be cool to have the output length, so we sould use
   * 0-terminated lines, thus no weird filename convention */
  
  GguGitShow *self = GGU_GIT_SHOW (obj);
  gchar     **lines;
  guint       i;
  GList      *entries = NULL;
  
  lines = g_strsplit (output, "\n", -1);
  for (i = 0; lines[i] != NULL; i++) {
    gchar **line;
    guint   n_added;
    guint   n_removed;
    GError *error = NULL;
    
    if (g_cancellable_set_error_if_cancelled (cancellable, &error)) {
      g_simple_async_result_take_error (result, error);
      break;
    }
    
    /* empty line, skip it */
    if (lines[i][0] == 0) {
      continue;
    }
    
    line = g_strsplit (lines[i], "\t", 3);
    if (! line[0] || ! line[1] || ! line[2]) {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Invalid output");
    } else if (parse_changes_count (line[0], &n_added, result) &&
               parse_changes_count (line[1], &n_removed, result) &&
               unescape_filename (line[2], result)) {
      GguGitFilesChangedEntry *entry = NULL;
      
      entry = ggu_git_files_changed_entry_new ();
      entry->hash     = g_strdup (self->priv->rev);
      entry->added    = n_added;
      entry->removed  = n_removed;
      entry->path     = line[2];
      
      entries = g_list_prepend (entries, entry);
      
      /* avoid freeing the filename. It's not a problem to set it to null
       * since it is the last strv entry */
      line[2] = NULL;
    }
    g_strfreev (line);
  }
  g_strfreev (lines);
  entries = g_list_reverse (entries);
  g_simple_async_result_set_op_res_gpointer (result, entries,
                                             (GDestroyNotify) files_changed_entry_list_unref);
}

void
ggu_git_list_files_changed_async (GguGitShow          *self,
                                  const gchar         *dir,
                                  const gchar         *rev,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  const gchar *argv[] = {
    "git",
    "show",
    "--numstat",
    "--format=%N",
    NULL, /* placeholder for rev */
    NULL
  };
  
  argv[G_N_ELEMENTS (argv) - 2] = rev;
  
  g_object_set (self,
                "dir", dir,
                "rev", rev,
                "file", NULL,
                "diff", FALSE,
                NULL);
  
  _ggu_git_run_async (GGU_GIT (self), (gchar **) argv,
                      ggu_git_list_files_changed_parse_output,
                      G_PRIORITY_DEFAULT, cancellable, callback, user_data);
}

GList *
ggu_git_list_files_changed_finish (GguGitShow    *self,
                                   GAsyncResult  *result,
                                   GError       **error)
{
  return _ggu_git_run_finish (GGU_GIT (self), result, error);
}



static const gchar *
skip_hash (const gchar *line)
{
  register const gchar *p = line;
  
  while (g_ascii_isxdigit (*p)) {
    p++;
  }
  
  if (p - line == 40 && (! *p || g_ascii_isspace (*p))) {
    return p;
  } else {
    return NULL;
  }
}

static void
entry_list_unref (GList *entries)
{
  g_list_free_full (entries, (GDestroyNotify) ggu_git_blame_entry_unref);
}

static void
ggu_git_blame_parse_output (GguGit             *obj,
                            const gchar        *output,
                            GSimpleAsyncResult *result,
                            GCancellable       *cancellable)
{
  /* Format:
   * 
   * <hash> <rigline> <line> <n_following>
   * author <name>
   * author-mail <email>
   * ... <many headers>
   * \t<line content>
   * 
   * the headers may be omitted if they are the same as the previous ones
   */
  
  gchar **lines = g_strsplit (output, "\n", 0);
  gchar **p;
  glong   count = 0;
  GList  *entries = NULL;
  
  for (p = lines; *p; p++) {
    const gchar      *line = skip_hash (*p);
    gchar            *end;
    GguGitBlameEntry *entry;
    
    if (! line) {
      continue;
    }
    
    entry = ggu_git_blame_entry_new ();
    entries = g_list_prepend (entries, entry);
    entry->hash = g_strndup (*p, (gsize) (line - *p));
    
    if (--count > 0) {
      const GguGitBlameEntry *old = entries->next->data;
      
      if (strcmp (entry->hash, old->hash) != 0) {
        g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                         GGU_GIT_ERROR_INVALID_RESULT,
                                         "Corrupted output: grouped commits with different hashes");
        break;
      }
      
      entry->line = old->line + 1;
      entry->author = g_strdup (old->author);
      continue;
    }
    
    
    while (*(++line) != ' '); /* oldline */
    entry->line = strtoul (line, &end, 0); /* line*/
    if (line == end) {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Corrupted output: missing line number");
      break;
    }
    line = end;
    
    count = strtol (line, &end, 10);
    if (line == end || *end != 0) {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Corrupted output: missing commit count");
      break;
    }
    
    /* now get the author name */
    line = *(++p);
    if (! line) {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Corrupted output: truncated data");
      break;
    }
    
    if (g_str_has_prefix (line, "author ")) {
      entry->author = ggu_git_utf8_ensure_valid (line + 7);  /* skip "author " */
    } else if (entries) {
      const GguGitBlameEntry *old = entries->next->data;
      
      entry->author = g_strdup (old->author);
    } else {
      g_simple_async_result_set_error (result, GGU_GIT_ERROR,
                                       GGU_GIT_ERROR_INVALID_RESULT,
                                       "Corrupted output: missing author info");
      break;
    }
  }
  
  entries = g_list_reverse (entries);
  g_simple_async_result_set_op_res_gpointer (result, entries,
                                             (GDestroyNotify) entry_list_unref);
  g_strfreev (lines);
}

/**
 * ggu_git_blame_async:
 * @self: A #GguGitShow object
 * @dir: Directory to run in
 * @rev: Revision to blame, or %NULL for HEAD
 * @file: File to blame
 * @cancellable: A #GCancellable object, or %NULL
 * @callback: The callback to be called when the operation result is ready
 * @user_data: User data for @callback
 * 
 * Performs a `git blame` inside @dir for file @file at revision @rev.
 * 
 * @callback can obtain the operation result using ggu_git_blame_finish().
 */
void
ggu_git_blame_async (GguGitShow          *self,
                     const gchar         *dir,
                     const gchar         *rev,
                     const gchar         *file,
                     GCancellable        *cancellable,
                     GAsyncReadyCallback  callback,
                     gpointer             user_data)
{
  const gchar *argv[] = {
    "git",
    "blame",
    /*"-p",*/ /* porcelain is buggy, sometimes it forgets to output
               * headers although that line doesn't has the same headers than
               * the previous one.  so, use --line-porcelain */
    "--line-porcelain",
    NULL, /* rev */
    NULL, /* -- */
    NULL, /* file */
    NULL
  };
  guint i = 3;
  
  g_return_if_fail (file != NULL);
  
  if (rev) {
    argv[i++] = rev;
  }
  argv[i++] = "--";
  argv[i++] = file;
  
  g_object_set (self,
                "dir", dir,
                "rev", rev,
                "file", file,
                "diff", FALSE,
                NULL);
  
  _ggu_git_run_async (GGU_GIT (self), (gchar **) argv,
                      ggu_git_blame_parse_output,
                      G_PRIORITY_DEFAULT, cancellable, callback, user_data);
}

/**
 * ggu_git_blame_finish:
 * @self: The #GguGitShow object that launched the operation
 * @result: The #GAsyncResult of the operation
 * @error: Return location for errors or %NULL to ignore
 * 
 * Gets the result of a blame operation.
 * 
 * Returns: (transfer none) (element-type GguGitBlameEntry): The list of blame
 *                                                           entries.
 */
GList *
ggu_git_blame_finish (GguGitShow   *self,
                      GAsyncResult *result,
                      GError      **error)
{
  return _ggu_git_run_finish (GGU_GIT (self), result, error);
}
