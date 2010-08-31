
#include "git-utils.h"

#include <glib.h>


void
git_list_free_full (GList      *l,
                    GFreeFunc   f)
{
  while (l) {
    GList *next = l->next;
    
    f (l->data);
    g_list_free_1 (l);
    l = next;
  }
}
