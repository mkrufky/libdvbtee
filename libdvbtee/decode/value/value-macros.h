/*****************************************************************************
 * Copyright (C) 2011-2015 Michael Ira Krufky
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

#ifndef __VALUE_MACROS_H__
#define __VALUE_MACROS_H__

#define GET_DEFAULT(CLASS, IN, OUT) \
template <> OUT& CLASS::get<OUT>(IN in) \
{ \
	OUT def; \
	return get<OUT>(in, def); \
}

#define GET_DEFAULT_VALUE(CLASS, IN, OUT, DEFAULT) \
template <> OUT& CLASS::get<OUT>(IN in) \
{ \
	OUT def = DEFAULT; \
	return get<OUT>(in, def); \
}

#define DEFINE_DEFAULT_GETTERS(CLASS, IN) \
namespace dvbtee { \
namespace decode { \
	GET_DEFAULT_VALUE(CLASS, IN, int, 0) \
	GET_DEFAULT_VALUE(CLASS, IN, long, 0) \
	GET_DEFAULT_VALUE(CLASS, IN, unsigned short, 0) \
	GET_DEFAULT_VALUE(CLASS, IN, unsigned char, 0) \
	GET_DEFAULT_VALUE(CLASS, IN, std::string, "") \
	GET_DEFAULT_VALUE(CLASS, IN, bool, false) \
	GET_DEFAULT_VALUE(CLASS, IN, double, 0.0) \
	GET_DEFAULT(CLASS, IN, Array) \
	GET_DEFAULT(CLASS, IN, Object) \
}}

#endif /* __VALUE_MACROS_H__ */
