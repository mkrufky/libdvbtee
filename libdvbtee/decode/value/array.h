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

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <map>
#include <string>
#include <vector>

#include "value.h"
#include "object.h"

namespace dvbtee {

namespace decode {

typedef std::vector<ValueBase*> KeyValueVector;

class Object;

class Array {
public:
	Array();
	~Array();

	Array(const Array&);

	void push(ValueBase*);

	template <ValueBase::Type TYPE, typename T>
	void push(T& val)
	{
		push(new Value<TYPE, T>(val));
	}

#if 0
	template <typename T> void push(T val);
#else
	inline void push(int val)
	{
		push<ValueBase::INTEGER, int>(val);
	}

	inline void push(unsigned short val)
	{
		push<ValueBase::UNSIGNEDSHORT, unsigned short>(val);
	}

	inline void push(unsigned char val)
	{
		push<ValueBase::UNSIGNEDCHAR, unsigned char>(val);
	}

	inline void push(std::string val)
	{
		push<ValueBase::STRING, std::string>(val);
	}

	inline void push(bool val)
	{
		push<ValueBase::BOOLEAN, bool>(val);
	}

	inline void push(double val)
	{
		push<ValueBase::DOUBLE, double>(val);
	}
#endif

	inline void push(Object& val)
	{
		push<ValueBase::OBJECT, Object>(val);
	}

	inline void push(Array& val)
	{
		push<ValueBase::ARRAY, Array>(val);
	}

	std::string toJson();

private:
	KeyValueVector vector;
};

}

}

#endif /* __ARRAY_H__ */
