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

#ifndef H_GGU_GIT_LOG_ENTRY
#define H_GGU_GIT_LOG_ENTRY

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS


#define GGU_TYPE_GIT_LOG_ENTRY (ggu_git_log_entry_get_type ())


typedef struct _GguGitLogEntry GguGitLogEntry;
struct _GguGitLogEntry
{
  gint      ref_count;
  
  /*guint64   hash;*/
  gchar    *hash;
  gchar    *date; /* TODO: use a date repsentation? */
  gchar    *author;
  gchar    *summary;
  gchar    *details;
};


GType             ggu_git_log_entry_get_type  (void) G_GNUC_CONST;
GguGitLogEntry   *ggu_git_log_entry_new       (void);
GguGitLogEntry   *ggu_git_log_entry_ref       (GguGitLogEntry *entry);
void              ggu_git_log_entry_unref     (GguGitLogEntry *entry);


G_END_DECLS

#endif /* guard */
