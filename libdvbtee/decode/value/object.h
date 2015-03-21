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

	template <typename T>
	void setByRef(std::string& key, T& val)
	{
		if (map.count(key))
			delete map[key];

		map[key] = new Value<T>(key, val);
		//map.insert( std::pair<std::string, ValueBase*>(key, new Value<T>(key, val)) );
	}

	template <typename T>
	void set(std::string key, T val)
	{
		setByRef<T>(key, val);
	}

	inline void set(std::string key,       char* val)	{ set<std::string>(key, std::string(val)); }
	inline void set(std::string key, const char* val)	{ set<std::string>(key, std::string(val)); }
	inline void set(std::string key, std::string& val)	{ setByRef<std::string>(key, val); }
	inline void set(std::string key, Array& val)		{ setByRef<Array>(key, val); }
	inline void set(std::string key, Object& val)		{ setByRef<Object>(key, val); }

	template <typename T>
	void set(int key, T val)
	{
		set(intToStr(key), val);
	}

	void set(ValueBase*);

	void unSet(std::string key);

	inline void unSet(int key)
	{
		unSet(intToStr(key));
	}

	ValueBase* get(std::string key);

	template <typename T>
	T& get(std::string& key, T& def)
	{
		if (map.count(key)) {
			Value<T> *val = (Value<T>*)map[key];
			if (val->checkType(typeid(T)))
				return val->get();
		}
		return def;
	}

	template <typename T> T& get(std::string key);

	inline ValueBase* get(int key)
	{
		return get(intToStr(key));
	}

	template <typename T>
	T& get(int key)
	{
		return get<T>(intToStr(key));
	}

	const std::string toJson();

private:
	KeyValueMap map;

	std::string intToStr(int);
};

}

}

#endif /* __OBJECT_H__ */
