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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <map>
#include <string>
#include <vector>

#include "value.h"
#include "array.h"

namespace dvbtee {

namespace decode {

typedef std::map<std::string, ValueBase*> KeyValueMap;

class Array;

class Object {
public:
	Object();
	~Object();

	Object(const Object&);

	template <ValueBase::Type TYPE, typename T>
	void set(std::string& key, T& val)
	{
		if (map.count(key))
			delete map[key];

		map[key] = new Value<TYPE, T>(key, val);
	}

	template<typename T> void set(std::string key, T val);
#if 0
	inline void set(std::string key, int val)
	{
		return set<ValueBase::INTEGER, int>(key, val);
	}

	inline void set(std::string key, std::string val)
	{
		return set<ValueBase::STRING, std::string>(key, val);
	}

	inline void set(std::string key, bool val)
	{
		return set<ValueBase::BOOLEAN, bool>(key, val);
	}

	inline void set(std::string key, double val)
	{
		return set<ValueBase::DOUBLE, double>(key, val);
	}
#endif
	inline void set(std::string key, Object& val)
	{
		return set<ValueBase::OBJECT, Object>(key, val);
	}

	inline void set(std::string key, Array& val)
	{
		return set<ValueBase::ARRAY, Array>(key, val);
	}

	void set(ValueBase*);

	ValueBase* get(std::string key);

	template <ValueBase::Type TYPE, typename T>
	T& get(std::string& key, T& def)
	{
		if (map.count(key)) {
			Value<TYPE, T> *val = (Value<TYPE, T>*)map[key];
			if (TYPE == val->type)
				return val->get();

			badType(TYPE, val);
		}

		return def;
	}

	template <typename T> T& get(std::string key);

	std::string toJson();

private:
	void badType(ValueBase::Type, ValueBase*);

	KeyValueMap map;
};

}

}

#endif /* __OBJECT_H__ */
