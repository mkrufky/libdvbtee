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

namespace valueobj {

class ValueBase {
public:
	ValueBase& operator++();
	ValueBase& operator--();

	virtual ~ValueBase();

	bool checkType(const std::type_info&) const;

	const std::type_info& getType() const;
	const std::string& getName() const;
	const int& getRefCnt() const;

	virtual const std::string toJson() const = 0;

protected:
	ValueBase(const std::type_info&, std::string);

private:
	const std::type_info& m_type;
	const std::string m_name;
	int m_refcnt;
};

#define VALUEBASE_POINTER 0
template <typename T>
class Value : public ValueBase {
public:
	explicit
	Value(std::string& n, T& v);

	Value(const Value<T>& o);

	~Value();

	const T& get() const;

	virtual const std::string toJson() const;

private:
#if VALUEBASE_POINTER
	const T *m_value;
#else
	const T  m_value;
#endif
};

class ValueUndefined : public ValueBase {
public:
	ValueUndefined(std::string n = "");
	explicit ValueUndefined(const ValueUndefined&);
	~ValueUndefined();
	const std::string get() const;
	virtual const std::string toJson() const;
};

class ReferencedValueUndefined : public ValueUndefined {
public:
	static ReferencedValueUndefined& instance();
private:
	ReferencedValueUndefined();
	~ReferencedValueUndefined();
};

}

#endif /* __VALUE_H__ */
