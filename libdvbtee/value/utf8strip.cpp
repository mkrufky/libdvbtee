/*****************************************************************************
 * Copyright (C) 2011-2018 Michael Ira Krufky
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

#include <locale>
#include <codecvt>
#include <string.h>
#include "utf8strip.h"

void wstrip(wchar_t * str)
{
	unsigned wchar_t *ptr, *s = (unsigned wchar_t*)str;
	ptr = s;
	while (*s != '\0') {
		if ((int)*s >= 0x20)
			*(ptr++) = *s;
		s++;
	}
	*ptr = '\0';
}

std::wstring_convert< std::codecvt_utf8_utf16<wchar_t> > converter;

std::string wstripped(std::string in)
{
	std::wstring win = converter.from_bytes(in);
	size_t len = win.length()*sizeof(wchar_t);
	wchar_t out[len] = { 0 };
	memcpy((void*)out, win.data(), len);
	out[len] = 0;
	wstrip(out);
	std::wstring wout = std::wstring(&out[0]);
	std::string retval = converter.to_bytes(wout);
	return retval;
}
