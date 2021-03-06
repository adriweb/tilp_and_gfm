/*
  Name: Group File Manager
  Copyright (C) 2006 Tyler Cassidy, Romain Lievin, Kevin Kofler
  04/06/06 17:04 - gui.h
  
  This program is free software you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef __GFMGUI_H__
#define __GFMGUI_H__

#include "support.h"

/* Structures */
typedef struct 
{
	GtkWidget *tree;
	GtkWidget *model;
	GtkWidget *entries;
	GtkWidget *comment;
	GtkWidget *ram;
	GtkWidget *flash;
	GtkWidget *save;
	GtkWidget *pbar;

	GList	*sel1;	// vars
	GList	*sel2;	// apps
} GFMWidget; 
extern GFMWidget gfm_widget;

/* Prototypes */
int launch_gfmgui(void);

void enable_save(int state);
void enable_tree(int state);

GLADE_CB void on_new_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_save_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_quit_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_gfm_dbox_destroy(GtkObject *object, gpointer user_data);
GLADE_CB gboolean on_gfm_dbox_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
GLADE_CB void on_add_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_delete_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_mkdir_clicked(GtkToolButton *toolbutton, gpointer user_data);
GLADE_CB void on_group_clicked(GtkButton *toolbutton, gpointer user_data);
GLADE_CB void on_ungroup_clicked(GtkButton *toolbutton, gpointer user_data);

#endif
