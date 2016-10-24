/*  XMMS
 *
 *  Copyright (C) 2006-2010 Teru KAMOGASHIRA
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _XMMS_DEFS_H
#define _XMMS_DEFS_H

#ifdef XMMS
#include <xmms/plugin.h>
#include <xmms/util.h>
#include <xmms/configfile.h>
#endif
#ifdef AUDACIOUS
#ifdef BEEPMEDIA
#include <bmp/plugin.h>
#include <bmp/util.h>
#include <bmp/configdb.h>
#else
#ifdef  __cplusplus
extern "C" {
#endif
#include <audacious/plugin.h>
#include <audacious/configdb.h>
#ifdef  __cplusplus
}
#endif
#endif
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
#ifdef  __cplusplus
extern "C" {
#endif
#include <libaudgui/libaudgui-gtk.h>
#ifdef  __cplusplus
}
#endif
#endif

#ifdef XMMS
#define _XMMS_DIALOG xmms_show_message
#ifdef AUDACIOUS140
#undef AUDACIOUS140
#endif
#endif

#ifdef BEEPMEDIA
#define _XMMS_DIALOG bmp_info_dialog
#ifdef AUDACIOUS140
#undef AUDACIOUS140
#endif
#endif

#ifdef AUDACIOUS
#ifndef BEEPMEDIA

#ifdef AUDACIOUS140
#if __AUDACIOUS_PLUGIN_API__ >= 16
#define _XMMS_DIALOG
#else
#define _XMMS_DIALOG audacious_info_dialog
#endif
#else
#define _XMMS_DIALOG bmp_info_dialog
#endif

#endif
#endif

#endif
