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
#include <vector>
#include <typeinfo>

namespace dvbtee {

namespace decode {

class ValueBase {
public:
	virtual ~ValueBase();

	const bool checkType(const std::type_info&) const;
	const std::string toJson() const;

	const std::type_info& getType() const;
	const std::string& getName() const;

protected:
	ValueBase(const std::type_info&, std::string);

private:
	const std::type_info& m_type;
	const std::string m_name;
};

#define VALUEBASE_POINTER 0
template <typename T>
class Value : public ValueBase {
public:
	Value(std::string& n, T& v)
	 : ValueBase(typeid(T), n)
#if !VALUEBASE_POINTER
	 , m_value(v)
#endif
	{
#if VALUEBASE_POINTER
		m_value = new T(v);
#endif
	}

	Value(const Value<T>& o)
	 : ValueBase(typeid(T), o.getName())
#if !VALUEBASE_POINTER
	 , m_value(o.get())
#endif
	{
#if VALUEBASE_POINTER
		m_value = new T(o.get());
#endif
	}

	~Value()
	{
#if VALUEBASE_POINTER
		delete m_value;
#endif
	}

	const T& get() const
	{
		if (!checkType(typeid(T))) {}
		return
#if VALUEBASE_POINTER
		*
#endif
		m_value;
	}

	void set(T& v)
	{
#if VALUEBASE_POINTER
		*
#endif
		m_value = v;
	}

private:
	T
#if VALUEBASE_POINTER
	*
#endif
	m_value;
};

class ValueUndefined : public ValueBase {
public:
	ValueUndefined(std::string n = "");
	ValueUndefined(const ValueUndefined&);
	~ValueUndefined();
	std::string get();
};

}

}

#endif /* __VALUE_H__ */
