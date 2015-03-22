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

	ValueBase* push(ValueBase*);

	template <typename T>
	ValueBase* push(T val)
	{
		return push<T>(val, "");
	}

	ValueBase* get(unsigned int idx);

	template <typename T> T& get(unsigned int idx);

	void clear();

	const size_t size();

	const std::string toJson();

private:
	KeyValueVector vector;

	template <typename T>
	ValueBase* pushByRef(T& val, std::string idx)
	{
		Value<T> *v = new Value<T>(idx, val);
		vector.push_back(v);
		return v;
	}

	template <typename T>
	ValueBase* push(T val, std::string idx)
	{
		return pushByRef<T>(val, idx);
	}

	inline ValueBase* push(      char* val, std::string idx)	{ return push<std::string>(std::string(val), idx); }
	inline ValueBase* push(const char* val, std::string idx)	{ return push<std::string>(std::string(val), idx); }
	inline ValueBase* push(std::string& val, std::string idx)	{ return pushByRef<std::string>(val, idx); }
	inline ValueBase* push(Array& val, std::string idx)		{ return pushByRef<Array>(val, idx); }
	inline ValueBase* push(Object& val, std::string idx)		{ return pushByRef<Object>(val, idx); }
	inline ValueBase* push(Array* val, std::string idx)		{ return pushByRef<Array>(*val, idx); }
	inline ValueBase* push(Object* val, std::string idx)		{ return pushByRef<Object>(*val, idx); }

	template <typename T>
	T& get(unsigned int &idx, T& def)
	{
		if (idx <= vector.size()) {
			Value<T> *val = (Value<T>*)vector[idx];
			if (val->checkType(typeid(T)))
				return val->get();
		}
		return def;
	}
};

}

}

#endif /* __ARRAY_H__ */
