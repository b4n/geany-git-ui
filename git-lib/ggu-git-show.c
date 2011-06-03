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

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ggu-git.h"


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
