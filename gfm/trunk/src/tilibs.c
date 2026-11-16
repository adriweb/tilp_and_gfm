/*
  Name: Group File Manager
  Copyright (C) 2006 Tyler Cassidy, Romain Lievin, Kevin Kofler
  04/06/06 16:22 - tilibs.c

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

/*
	TiLP framework initialization.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>

#include "dialog.h"
#include "tilibs.h"
#include "version.h"
#include "support.h"

// Load Tilibs */
int load_tilibs(void)
{
#if defined(__WIN32__) && (defined(_MSC_VER) || defined(__DEVCPP__))
  // Check libticonv
  if (g_ascii_strcasecmp(ticonv_version_get(), GFM_REQUIRES_LIBTICONV_VERSION) < 0)
  {
    gchar *error_msg;
    error_msg = g_strdup_printf(_("This application requires %s version <b>%s</b>.\n"
                                 "You have version <b>%s</b>.\nPlease upgrade %s and try again."),
                                "libticonv", GFM_REQUIRES_LIBTICONV_VERSION,
                                ticonv_version_get(), "libticonv");
    msgbox_error(error_msg);
    g_free(error_msg);
    exit(-1);
  }
	
  // Check libtifiles2
  if (g_ascii_strcasecmp(tifiles_version_get(), GFM_REQUIRES_LIBTIFILES2_VERSION) < 0)
  {
    gchar *error_msg;
    error_msg = g_strdup_printf(_("This application requires %s version <b>%s</b>.\n"
                                 "You have version <b>%s</b>.\nPlease upgrade %s and try again."),
                                "libtifiles2", GFM_REQUIRES_LIBTIFILES2_VERSION,
                                tifiles_version_get(), "libtifiles2");
    msgbox_error(error_msg);
    g_free(error_msg);
    exit(-1);
  }

  // Check libticalcs2
  if (g_ascii_strcasecmp(ticalcs_version_get(), GFM_REQUIRES_LIBTICALCS2_VERSION) < 0)
  {
    gchar *error_msg;
    error_msg = g_strdup_printf(_("This application requires %s version <b>%s</b>.\n"
                                 "You have version <b>%s</b>.\nPlease upgrade %s and try again."),
                                "libticalcs2", GFM_REQUIRES_LIBTICALCS2_VERSION,
                                ticalcs_version_get(), "libticalcs2");
    msgbox_error(error_msg);
    g_free(error_msg);
    exit(-1);
  }	
#endif // End of Windows Extravaganza.

  // Load libtifiles2 and libticalcs2
  tifiles_library_init();
  ticalcs_library_init();
	
  // Return
  return 0;
}
