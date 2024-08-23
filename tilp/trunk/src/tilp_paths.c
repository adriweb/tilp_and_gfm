/* Hey EMACS -*- linux-c -*- */
/* $Id$ */

/*  TiLP - Tilp Is a Linking Program
 *  Copyright (C) 1999-2006  Romain Lievin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
	Initialization of portable paths
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if defined(__MACOSX__)
#include <mach-o/dyld.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include "tilp_core.h"

#ifndef GSETTINGS_SCHEMA_SUBDIR
#define GSETTINGS_SCHEMA_SUBDIR "share/tilp2/glib_schemas"
#endif

TilpInstPaths inst_paths = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

/*
  Called by TiLP at startup for initializing platform dependant paths.
*/
#if defined(__LINUX__) || defined(__BSD__) || defined(__MACOSX__)
static void init_linux_paths(void)
{
#ifdef RELOCATABLE_INSTALL
	gchar *executable_path = g_file_read_link("/proc/self/exe", NULL);
	gchar *bin_dir = executable_path ? g_path_get_dirname(executable_path) : NULL;
	gchar *prefix_dir = bin_dir ? g_path_get_dirname(bin_dir) : NULL;

	inst_paths.base_dir = prefix_dir
	    ? g_strconcat(prefix_dir, G_DIR_SEPARATOR_S, SHARE_SUBDIR, G_DIR_SEPARATOR_S, NULL)
	    : g_strconcat(SHARE_DIR, G_DIR_SEPARATOR_S, NULL);
#else
	inst_paths.base_dir =
	    g_strconcat(SHARE_DIR, G_DIR_SEPARATOR_S, NULL);
#endif
	inst_paths.pixmap_dir =
	    g_strconcat(inst_paths.base_dir, "pixmaps/", NULL);
	inst_paths.icon_dir =
	    g_strconcat(inst_paths.base_dir, "icons/", NULL);
	inst_paths.help_dir =
	    g_strconcat(inst_paths.base_dir, "help/", NULL);
	inst_paths.manpage_dir = 
            g_strconcat(inst_paths.base_dir, "", NULL);
	inst_paths.glade_dir =
	    g_strconcat(inst_paths.base_dir, "glade/", NULL);
	inst_paths.builder_dir =
	    g_strconcat(inst_paths.base_dir, "builder/", NULL);
	inst_paths.home_dir =
		g_strdup(g_get_home_dir());

#ifdef ENABLE_NLS
#ifdef RELOCATABLE_INSTALL
	inst_paths.locale_dir = prefix_dir
	    ? g_strconcat(prefix_dir, G_DIR_SEPARATOR_S, LOCALE_SUBDIR, G_DIR_SEPARATOR_S, NULL)
	    : g_strconcat(LOCALEDIR, "/", NULL);
#else
	inst_paths.locale_dir = g_strconcat(LOCALEDIR, "/", NULL);
#endif
#endif				/*  */
#ifdef RELOCATABLE_INSTALL
	g_free(prefix_dir);
	g_free(bin_dir);
	g_free(executable_path);
#endif
}
#endif				/*  */

#ifdef __WIN32__
static void init_win32_paths(void)
{
	char *home_dir = NULL;
	HMODULE hModule;
	DWORD dWord;
	char *dirname;
	char *sBuffer;

	// Init the path for the Windows version by getting the 
	// executable location.
	hModule = GetModuleHandle(NULL);
	sBuffer = (char *) malloc(4096 * sizeof(char));
	dWord = GetModuleFileName(hModule, sBuffer, 4096);
	dirname = g_dirname(sBuffer);

	// MinGW Option, allows Windows Users to run on a Linux File Hierarhcy
	/*#ifdef __MINGW32__
	  #define MINGW_REL "share\\tilp2"
	  
	  char *basename;
	  basename = g_path_get_basename(dirname);
	  
      // Will replace /target/bin with /target/share/tilp2 in MinGW/MSYS
      if ((strlen(basename) == 3) && !g_ascii_strcasecmp(basename, "bin"))
      {
          gchar *token;
          dirname = g_realloc(dirname, strlen(dirname) + strlen(MINGW_REL) + 1);
          token = dirname + strlen(dirname) - 3;
          strcpy(token, MINGW_REL);
      }
    #endif*/
    
    inst_paths.base_dir = g_strconcat(dirname, "\\", NULL);
	g_free(dirname);
	free(sBuffer);

	inst_paths.pixmap_dir =
	    g_strconcat(inst_paths.base_dir, "pixmaps\\", NULL);
	inst_paths.icon_dir =
	    g_strconcat(inst_paths.base_dir, "icons\\", NULL);
	inst_paths.help_dir =
	    g_strconcat(inst_paths.base_dir, "help\\", NULL);
	inst_paths.manpage_dir =
	    g_strconcat(inst_paths.base_dir, "", NULL);
	inst_paths.glade_dir =
	    g_strconcat(inst_paths.base_dir, "glade\\", NULL);
	inst_paths.builder_dir =
	    g_strconcat(inst_paths.base_dir, "builder\\", NULL);

	inst_paths.home_dir = g_strconcat(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS), G_DIR_SEPARATOR_S, _("My TI Files"), NULL);
	g_mkdir_with_parents(inst_paths.home_dir, 0755);


#ifdef ENABLE_NLS
	inst_paths.locale_dir =
	    g_strconcat(inst_paths.base_dir, "locale\\", NULL);
#endif				/*  */
}
#endif				/*  */

#if defined(__MACOSX__) || defined(__LINUX__) || defined(__WIN32__)
/*
 * GTK 3's file chooser uses GSettings. A static GTK build still needs its
 * compiled schemas at runtime, so point GLib at the copy installed next to
 * TiLP. Keep an explicit user override, and retain GLib's normal search path
 * when the bundled database is not present (for example, in a build tree).
 */
static gchar *get_executable_path(void)
{
#if defined(__MACOSX__)
	uint32_t executable_path_size = 0;
	gchar *executable_path;

	_NSGetExecutablePath(NULL, &executable_path_size);
	if (executable_path_size == 0)
	{
		return NULL;
	}

	executable_path = g_malloc(executable_path_size);
	if (_NSGetExecutablePath(executable_path, &executable_path_size) != 0)
	{
		g_free(executable_path);
		return NULL;
	}

	return executable_path;
#elif defined(__LINUX__)
	return g_file_read_link("/proc/self/exe", NULL);
#elif defined(__WIN32__)
	DWORD executable_path_size = MAX_PATH;

	while (executable_path_size <= 32768)
	{
		DWORD length;
		gunichar2 *executable_path_utf16 = g_new(gunichar2, executable_path_size);
		gchar *executable_path;

		length = GetModuleFileNameW(NULL, (LPWSTR) executable_path_utf16,
		                            executable_path_size);
		if (length == 0)
		{
			g_free(executable_path_utf16);
			return NULL;
		}
		if (length < executable_path_size)
		{
			executable_path = g_utf16_to_utf8(executable_path_utf16,
			                                  length, NULL, NULL, NULL);
			g_free(executable_path_utf16);
			return executable_path;
		}

		g_free(executable_path_utf16);
		executable_path_size *= 2;
	}

	return NULL;
#endif
}

static void init_bundled_gsettings_path(void)
{
	gchar *executable_path;
	gchar *canonical_path;
	gchar *bin_dir;
	gchar *prefix_dir;
	gchar *schema_dir;
	gchar *compiled_schema;

	if (g_getenv("GSETTINGS_SCHEMA_DIR") != NULL)
	{
		return;
	}

	executable_path = get_executable_path();
	if (executable_path == NULL)
	{
		return;
	}

	canonical_path = g_canonicalize_filename(executable_path, NULL);
	bin_dir = g_path_get_dirname(canonical_path);
	prefix_dir = g_path_get_dirname(bin_dir);
	schema_dir = g_build_filename(prefix_dir, GSETTINGS_SCHEMA_SUBDIR, NULL);
	compiled_schema = g_build_filename(schema_dir, "gschemas.compiled", NULL);

	if (g_file_test(compiled_schema, G_FILE_TEST_IS_REGULAR))
	{
		g_setenv("GSETTINGS_SCHEMA_DIR", schema_dir, FALSE);
	}

	g_free(compiled_schema);
	g_free(schema_dir);
	g_free(prefix_dir);
	g_free(bin_dir);
	g_free(canonical_path);
	g_free(executable_path);
}
#endif

int tilp_paths_init(void)
{

#if defined(__MACOSX__) || defined(__LINUX__) || defined(__WIN32__)
	init_bundled_gsettings_path();
#endif

#if defined(__LINUX__) || defined(__BSD__) || defined(__MACOSX__)
	init_linux_paths();

#elif defined(__WIN32__)
	init_win32_paths();

#endif				/*  */
	return 0;
}

const char *tilp_paths_build_glade(const char *name)
{
	static char *path = NULL;

	g_free(path);
	path = g_strconcat(inst_paths.glade_dir, name, NULL);

	return path;
}

const char *tilp_paths_build_builder(const char *name)
{
	static char *path = NULL;

	g_free(path);
	path = g_strconcat(inst_paths.builder_dir, name, NULL);

	return path;
}
