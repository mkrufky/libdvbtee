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

#ifndef __VALUE_H__
#define __VALUE_H__

#include <map>
#include <string>

namespace dvbtee {

namespace decode {

class ValueBase {
public:
	enum Type {
		INTEGER,
		STRING,
		BOOLEAN,
		DOUBLE,
		OBJECT,
	};

	ValueBase(Type, std::string);
	virtual ~ValueBase();

	const Type type;

	const std::string name;
};

template <ValueBase::Type TYPE, typename T>
class Value : public ValueBase {
public:
	Value(std::string& n, T& v)
	 : ValueBase(TYPE, n)
	{
		pValue = new T(v);
	}

	~Value()
	{
		delete pValue;
	}

	T& get()
	{
		return *pValue;
	}

	void set(T& v)
	{
		*pValue = v;
	}

private:
	T *pValue;
};

typedef std::map<std::string, ValueBase*> KeyValueMap;

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

	inline void set(std::string key, Object val)
	{
		return set<ValueBase::OBJECT, Object>(key, val);
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

private:
	void badType(ValueBase::Type, ValueBase*);

	KeyValueMap map;
};

}

}

#endif /* __VALUE_H__ */
