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
#define USE_CODECVT 0

#include <locale>
#include <string.h>
#include "utf8strip.h"

#if USE_CODECVT
#include <codecvt>

std::wstring_convert< std::codecvt_utf8_utf16<wchar_t> > converter;
#else
#include <stdlib.h>
#include <vector>

wchar_t *to_wide(const char *str);
#endif

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

std::string wstripped(std::string in)
{
#if USE_CODECVT
	std::wstring win = converter.from_bytes(in);
#else
	wchar_t *_win = to_wide((char*)in.data());
	std::wstring win(_win);
	free(_win);
#endif
	size_t len = win.length()*sizeof(wchar_t);
#if 0
	wchar_t out[len] = { 0 };
	memcpy((void*)out, win.data(), len);
	out[len] = 0;
#else
	wchar_t *out = (wchar_t *)win.data();
#endif
	wstrip(out);

#if USE_CODECVT
	std::wstring wout = std::wstring(&out[0]);
	std::string retval = converter.to_bytes(wout);
	return retval;
#else
	// FIXME: read https://www.gnu.org/software/libc/manual/html_node/Setting-the-Locale.html
	setlocale(LC_ALL, ""); // FIXME: only run once & test in configure.ac

	std::mbstate_t state = std::mbstate_t();
	std::size_t olen = 1 + std::wcsrtombs(NULL, (const wchar_t**)&out, 0, &state);
	std::vector<char> mbstr(olen);
	std::wcsrtombs(&mbstr[0], (const wchar_t**)&out, mbstr.size(), &state); // FIXME: test in configure.ac
	return std::string(&mbstr[0]);
#endif
}
