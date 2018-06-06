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
#include <string.h>
#include "utf8strip.h"
#include "../dvbtee_config.h"

#ifndef USE_WSTRING_CONVERT
#define USE_WSTRING_CONVERT 0
#endif

#ifndef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 0
#if USE_WSTRING_CONVERT
#undef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 1
#endif /* USE_WSTRING_CONVERT */
#if defined(_WIN32)
#undef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 1
#endif /* defined(_WIN32) */
#endif /* USE_DVBTEE_WSTRIP */

#if !USE_DVBTEE_WSTRIP
/* FIXME:
 * v8::JSON::Parse blows up if we try to do this using anything other than
 * `std::wstring_convert`, so for now, provide a no-op wstripped() otherwise
 */

std::string wstripped(std::string in)
{
    return in;
}
#else
/* The outer `#if !USE_WSTRING_CONVERT` is a "temporary hack"
 * we check again below for compatibility
 */

#if USE_WSTRING_CONVERT
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
#if USE_WSTRING_CONVERT
    return converter.from_bytes(external);
#else
#if !defined(_WIN32)
    // from: http://en.cppreference.com/w/cpp/locale/codecvt/in
    std::locale::global(std::locale(""));
    const std::codecvt<wchar_t, char, std::mbstate_t> &f =
	  std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(std::locale());

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
#else
    // WINBASEAPI int WINAPI MultiByteToWideChar (UINT CodePage, DWORD dwFlags, LPCCH lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
    int Newlength = MultiByteToWideChar(CP_ACP, WC_COMPOSITECHECK, external.c_str(), -1, NULL, 0);
    std::wstring NewString(Newlength + 1, 0);
    int Newresult = MultiByteToWideChar(CP_ACP, WC_COMPOSITECHECK, external.c_str(), -1, &NewString[0],Newlength + 1);
	return NewString;
#endif
#endif
}

std::string to_narrow(std::wstring internal)
{
#if USE_WSTRING_CONVERT
    return converter.to_bytes(internal);
#else
#if !defined(_WIN32)
    // from: http://en.cppreference.com/w/cpp/locale/codecvt/out
    std::locale::global(std::locale(""));
    const std::codecvt<wchar_t, char, std::mbstate_t> &f =
	  std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t> >(std::locale());

    // note that the following can be done with wstring_convert
    std::mbstate_t mb = std::mbstate_t(); // initial shift state
    std::string external(internal.size() * f.max_length(), '\0');
    const wchar_t* from_next;
    char* to_next;
    f.out(mb, &internal[0], &internal[internal.size()], from_next,
              &external[0], &external[external.size()], to_next);
    // error checking skipped for brevity
    external.resize(to_next - &external[0]);
    return external;
#else
    int Newlength = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, internal.c_str(), -1, NULL, 0,  NULL, NULL);
    std::string NewString(Newlength + 1, 0);
    int Newresult = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, internal.c_str(), -1, &NewString[0],Newlength + 1,  NULL, NULL);
	return NewString;
#endif
#endif
}

std::string wstripped(std::string in)
{
    std::wstring win = to_wide(in);

    wchar_t *out = (wchar_t *)win.data();
    wstrip(out);

    return to_narrow(out);
}
#endif
