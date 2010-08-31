
#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "ggu-git-wrapper-log.h"
#include "ggu-git-wrapper-branch-list.h"

static GMainLoop *P_loop = NULL;
static gint       P_refs = 0;

static void
loop_push (void)
{
  P_refs++;
}

static void
loop_pop (void)
{
  if (--P_refs <= 0) {
    g_main_loop_quit (P_loop);
    g_main_loop_unref (P_loop);
    P_loop = NULL;
  }
}


static void
log_result_callback (GList         *commits,
                     const GError  *error,
                     gpointer       data)
{
  loop_pop ();
  
  if (error) {
    g_warning ("%s", error->message);
  } else {
    printf ("=== Commit(s) ===\n");
    for (; commits; commits = commits->next) {
      GguGitCommit *commit = commits->data;
      
      printf ("%.7s -- %s\n", commit->hash, commit->summary);
    }
  }
}

static void
branch_list_result_callback (GList         *branches,
                             const gchar   *current_branch,
                             const GError  *error,
                             gpointer       data)
{
  loop_pop ();
  
  if (error) {
    g_warning ("%s", error->message);
  } else {
    printf ("=== Branch(es) ===\n");
    for (; branches; branches = branches->next) {
      gchar *branch = branches->data;
      
      printf ("%c %s\n",
              strcmp (branch, current_branch) == 0 ? '*' : ' ', branch);
    }
  }
}

static int
ggu_git_wrapper_test_main (int     argc,
                           char  **argv)
{
  int rv = 1;
  
  loop_push ();
  if (argc == 2) {
    gchar *path;
    gchar *dir;
    gchar *file;
    
    path = g_strdup (argv[1]);
    dir = g_path_get_dirname (path);
    file = g_path_get_basename (path);
    loop_push ();
    ggu_git_log (dir, NULL, file, log_result_callback, NULL);
    loop_push ();
    ggu_git_branch_list (dir, branch_list_result_callback, NULL);
    rv = 0;
    
    {
      GList  *l;
      GError *err = NULL;
      
      l = ggu_git_log_sync (dir, NULL, file, &err);
      if (err) {
        g_warning ("%s", err->message);
        g_error_free (err);
      } else {
        printf ("=== Commit(s) ===\n");
        while (l) {
          GguGitCommit *commit = l->data;
          GList *next = l->next;
          
          printf ("%.7s -- %s\n", commit->hash, commit->summary);
          ggu_git_commit_unref (commit);
          g_list_free_1 (l);
          l = next;
        }
      }
    }
    
    g_free (file);
    g_free (dir);
    g_free (path);
  }
  loop_pop ();
  
  return rv;
}





int
main (int     argc,
      char  **argv)
{
  int rv;
  
  P_loop = g_main_loop_new (NULL, FALSE);
  rv = ggu_git_wrapper_test_main (argc, argv);
  if (rv == 0 && P_loop) {
    g_main_loop_run (P_loop);
  }
  
  return rv;
}
