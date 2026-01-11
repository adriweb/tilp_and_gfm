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
	GTK callbacks for ticalcs library (gui)
*/

#include <stdio.h>
#include <gtk/gtk.h>

#include "tilp_core.h"
#include "gstruct.h"
#include "gtk_update.h"

//#define DISABLE_UPDATE	// for testing purposes

static gfloat filter[8] = { 0 };
static gint64 rate_last_time = 0;
static gdouble rate_last_cnt1 = 0.0;
static gfloat rate_last_kbps = 0.0;

static void reset_rate_state(void)
{
	int i;

	rate_last_time = 0;
	rate_last_cnt1 = 0.0;
	rate_last_kbps = 0.0;
	for(i = 0; i < 8; i++)
		filter[i] = 0.0;
}

static void gtk_start(void)
{
	gtk_update.cnt1 = gtk_update.max1 = 0;
	gtk_update.cnt2 = gtk_update.max2 = 0;
	gtk_update.cnt3 = gtk_update.max3 = 0;
	reset_rate_state();
}

static void gtk_stop(void)
{
	gtk_update.cnt1 = gtk_update.max1 = 0;
	gtk_update.cnt2 = gtk_update.max2 = 0;
	gtk_update.cnt3 = gtk_update.max3 = 0;
	reset_rate_state();
}

static void filter_shift(void)
{
	int i;

	for(i=7; i>0; i--)
		filter[i] = filter[i-1];
}

static gfloat filter_compute(gfloat input)
{
	int i;
	gfloat avg, min, max;

	avg = min = max = 0.0;

	filter[0] = input;
	for(i=0; i<7; i++) {
		if(filter[i] < min) min = filter[i];
		if(filter[i] > max) max = filter[i];

		avg += filter[i];
	}

	avg -= min;
	avg -= max;

	return (avg / 6);
}

static gfloat compute_rate_kbps(gboolean *has_rate)
{
	gint64 now;
	gdouble elapsed;
	gdouble current;
	gdouble delta_bytes;

#if GLIB_CHECK_VERSION(2, 28, 0)
	now = g_get_monotonic_time();
#else
	{
		GTimeVal tv;
		g_get_current_time(&tv);
		now = ((gint64)tv.tv_sec * 1000000) + tv.tv_usec;
	}
#endif
	current = gtk_update.cnt1;

	if (rate_last_time == 0)
	{
		rate_last_time = now;
		rate_last_cnt1 = current;
		if (has_rate)
			*has_rate = FALSE;
		return rate_last_kbps;
	}

	elapsed = (gdouble)(now - rate_last_time) / 1000000.0;
	if (elapsed <= 0.0)
	{
		if (has_rate)
			*has_rate = (rate_last_kbps > 0.0);
		return rate_last_kbps;
	}

	delta_bytes = current - rate_last_cnt1;
	if (delta_bytes <= 0.0)
	{
		if (has_rate)
			*has_rate = (rate_last_kbps > 0.0);
		return rate_last_kbps;
	}

	rate_last_kbps = (gfloat)((delta_bytes / elapsed) / 1024.0);
	rate_last_time = now;
	rate_last_cnt1 = current;

	if (has_rate)
		*has_rate = TRUE;
	return rate_last_kbps;
}

static void refresh_pbar1(void)
{
	gchar buffer[64];
	gfloat rate, avg;
	gboolean has_rate;

	if (pbar_wnd.pbar1 != NULL) 
	{
		if(gtk_update.cnt1 > gtk_update.max1)
			gtk_update.cnt1 = gtk_update.max1;

		if(gtk_update.max1 != 0)
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar_wnd.pbar1), 
				(gdouble)gtk_update.cnt1 / gtk_update.max1);
		else
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(pbar_wnd.pbar1));

		rate = compute_rate_kbps(&has_rate);
		filter_shift();
		avg = filter_compute(rate);

		if (has_rate && gtk_update.max1 > 0 && avg > 0.0)
		{
			gdouble eta = (gtk_update.max1 - gtk_update.cnt1) / (avg * 1024.0);
			if (eta > 0.0)
			{
				int eta_i = (int)(eta + 0.5);
				int eta_m = eta_i / 60;
				int eta_s = eta_i % 60;
				g_snprintf(buffer, sizeof(buffer),
					"Rate: %1.1f Kbytes/s ETA: %02i:%02i", avg, eta_m, eta_s);
			}
			else
			{
				g_snprintf(buffer, sizeof(buffer), "Rate: %1.1f Kbytes/s", avg);
			}
		}
		else
		{
			g_snprintf(buffer, sizeof(buffer), "Rate: %1.1f Kbytes/s", avg);
		}
		gtk_label_set_text(GTK_LABEL(pbar_wnd.label_rate), buffer);

		GTK_REFRESH();
	}
}

static void refresh_pbar2(void)
{
	if (pbar_wnd.pbar2 != NULL) 
	{
		if(gtk_update.cnt2 > gtk_update.max2)
			gtk_update.cnt2 = gtk_update.max2;

		if(gtk_update.max2 != 0)
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar_wnd.pbar2), 
				(gdouble)gtk_update.cnt2 / gtk_update.max2);
		else
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(pbar_wnd.pbar2));

		GTK_REFRESH();
	}
}

static void refresh_pbar3(void)
{
	gchar buffer[32];

	if(pbar_wnd.label_part != NULL)
	{
		g_snprintf(buffer, 32, "%i/%i: ", gtk_update.cnt3, gtk_update.max3);
		gtk_label_set_text(GTK_LABEL(pbar_wnd.label_part), buffer);

		GTK_REFRESH();
	}
}

static void gtk_pbar(void)
{
#ifndef DISABLE_UPDATE
	refresh_pbar1();
	refresh_pbar2();
	refresh_pbar3();
#endif
} 

// note: info_update.label_text is encoded in UTF8 but variable names ('%s')
// are encoded according to tifiles_translate_set_encoding().
// This should be treated here but given that encoding is set to UTF8, there
// is nothing to do...
static void gtk_label(void)
{
#ifndef DISABLE_UPDATE
	if (pbar_wnd.label == NULL)
		return;

	gtk_label_set_text(GTK_LABEL(pbar_wnd.label), gtk_update.text);

	GTK_REFRESH();
#endif
}

static void gtk_refresh(void)
{
#ifndef DISABLE_UPDATE
	GTK_REFRESH();
#endif
}

CalcUpdate gtk_update =
{
	"", 0,
	0.0, 0, 0, 0, 0, 0, 0,  (1 << 0) | (1 << 1), 0,
	gtk_start,
	gtk_stop,
	gtk_refresh,
	gtk_pbar,
	gtk_label,
};

void tilp_update_set_gtk(void)
{
	ticalcs_update_set(calc_handle, &gtk_update);
}
