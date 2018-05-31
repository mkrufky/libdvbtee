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

std::wstring to_wide(std::string external)
{
    // from: http://en.cppreference.com/w/cpp/locale/codecvt/in
    std::locale::global(std::locale("en_US.utf8"));
    auto& f = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(std::locale());

    // note that the following can be done with wstring_convert
    std::mbstate_t mb = std::mbstate_t(); // initial shift state
    std::wstring internal(external.size(), '\0');
    const char* from_next;
    wchar_t* to_next;
    f.in(mb, &external[0], &external[external.size()], from_next,
             &internal[0], &internal[internal.size()], to_next);
    // error checking skipped for brevity
    internal.resize(to_next - &internal[0]);
    return internal;
}

std::string to_narrow(std::wstring internal)
{
    // from: http://en.cppreference.com/w/cpp/locale/codecvt/out
    std::locale::global(std::locale("en_US.utf8"));
    auto& f = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(std::locale());

    // note that the following can be done with wstring_convert
    std::mbstate_t mb{}; // initial shift state
    std::string external(internal.size() * f.max_length(), '\0');
    const wchar_t* from_next;
    char* to_next;
    f.out(mb, &internal[0], &internal[internal.size()], from_next,
              &external[0], &external[external.size()], to_next);
    // error checking skipped for brevity
    external.resize(to_next - &external[0]);
    return external;
}

std::string wstripped(std::string in)
{
#if USE_CODECVT
	std::wstring win = converter.from_bytes(in);
#else
	std::wstring win = to_wide(in);
#endif

	wchar_t *out = (wchar_t *)win.data();
	wstrip(out);

#if USE_CODECVT
	std::wstring wout = std::wstring(&out[0]);
	std::string retval = converter.to_bytes(wout);
	return retval;
#else
	return to_narrow(out);
#endif
}
