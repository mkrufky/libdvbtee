/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef __DVBTEE_CONFIG_H__
#define __DVBTEE_CONFIG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LINUX_DVB_FRONTEND_H
#if HAVE_LINUX_DVB_FRONTEND_H
#define USE_LINUXTV
#endif
#endif

#ifdef HAVE_LIBHDHOMERUN_HDHOMERUN_H
#if HAVE_LIBHDHOMERUN_HDHOMERUN_H
#define USE_HDHOMERUN
#endif
#endif

#ifdef HAVE_LIBHDHOMERUN
#if HAVE_LIBHDHOMERUN
#define USE_HDHOMERUN
#endif
#endif

#endif //__DVBTEE_CONFIG_H__
