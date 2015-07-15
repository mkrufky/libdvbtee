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

#include <stdio.h>
#include <sstream>
#include "object.h"
#include "value-macros.h"

using namespace valueobj;

DEFINE_DEFAULT_GETTERS(Object, std::string)

namespace valueobj {

template <typename T>
const ValueBase* Object::setByRef(std::string& key, T& val)
{
	if (map.count(key)) {
		if (0 == (--(*map[key])).getRefCnt()) delete map[key];
	}

	Value<T> *v = new Value<T>(key, val);

	map[key] = v;
	//map.insert( std::pair<std::string, ValueBase*>(key, v) );

	++(*v); // increment refcount
	return v;
}

template <typename T>
const T& Object::get(std::string& key, T& def) const
{
	KeyValueMap::const_iterator it = map.find(key);
	if (it != map.end()) {
		Value<T> *val = (Value<T>*)(it->second);
		if (val->checkType(typeid(T)))
			return val->get();
	}
	return def;
}
}

#define IMPL_OBJECT_TMPL(T) \
template const ValueBase* Object::setByRef<T>(std::string& key, T& val); \
template const T& Object::get<T>(std::string& key, T& def) const

IMPL_OBJECT_TMPL(int);
IMPL_OBJECT_TMPL(long);
IMPL_OBJECT_TMPL(long long);
IMPL_OBJECT_TMPL(short);
IMPL_OBJECT_TMPL(char);
IMPL_OBJECT_TMPL(unsigned int);
IMPL_OBJECT_TMPL(unsigned long);
IMPL_OBJECT_TMPL(unsigned long long);
IMPL_OBJECT_TMPL(unsigned short);
IMPL_OBJECT_TMPL(unsigned char);
IMPL_OBJECT_TMPL(std::string);
IMPL_OBJECT_TMPL(bool);
IMPL_OBJECT_TMPL(double);
IMPL_OBJECT_TMPL(Array);
IMPL_OBJECT_TMPL(Object);


#define DBG 0

static ReferencedValueUndefined& valueUndefined = ReferencedValueUndefined::instance();

Object::Object()
{
#if DBG
	fprintf(stderr, "%s\n", __func__);
#endif
}

Object::~Object()
{
#if DBG
	fprintf(stderr, "%s\n", __func__);
#endif
	clear();
}

Object::Object(const Object &obj)
{
	for (KeyValueMap::const_iterator it = obj.map.begin(); it != obj.map.end(); ++it) {
		set(it->second);
	}

#if DBG
	fprintf(stderr, "%s(copy) %lu\n", __func__, map.size());
#endif
}

const ValueBase *Object::set(std::string key, char *val)
{
	return set<std::string>(key, std::string(val));
}

const ValueBase *Object::set(std::string key, const char *val)
{
	return set<std::string>(key, std::string(val));
}

const ValueBase *Object::set(std::string key, std::string &val)
{
	return setByRef<std::string>(key, val);
}

const ValueBase *Object::set(std::string key, Array &val)
{
	return setByRef<Array>(key, val);
}

const ValueBase *Object::set(std::string key, Object &val)
{
	return setByRef<Object>(key, val);
}

const ValueBase *Object::set(std::string key, Array *val)
{
	return setByRef<Array>(key, *val);
}

const ValueBase *Object::set(std::string key, Object *val)
{
	return setByRef<Object>(key, *val);
}

const ValueBase* Object::get(std::string key) const
{
	KeyValueMap::const_iterator it = map.find(key);
	if (it != map.end())
		return it->second;

	return &valueUndefined;
}

const ValueBase* Object::get(int key) const
{
	return get(intToStr(key));
}

void Object::unSet(std::string key)
{
	if (map.count(key)) {
		if (0 == (--(*map[key])).getRefCnt()) delete map[key];
		map.erase(key);
	}
}

void Object::unSet(int key)
{
	unSet(intToStr(key));
}

void Object::clear()
{
	for (KeyValueMap::iterator it = map.begin(); it != map.end(); ++it)
	{
		// decrement refcount. if refcount becomes zero, delete
		if (0 == (--(*it->second)).getRefCnt()) delete it->second;
	}
	map.clear();
}

const std::string Object::toJson() const
{
	std::stringstream s;

	if (!map.size()) return "{}";

	int count = 0;

	s << "{ ";
	for (KeyValueMap::const_iterator it = map.begin(); it != map.end(); ++it) {
		ValueBase *val = it->second;

		if (count) s << ", ";
		s << "'" << it->first << "': ";

		s << val->toJson();

		count++;
	}
	s << " }";

	return s.str();
}

const std::string Object::intToStr(int i) const
{
	std::stringstream s;
	s << i;
	return s.str();
}

const ValueBase *Object::set(ValueBase *val)
{
	unSet(val->getName());
	map[val->getName()] = val;
	++(*val);
	return val;
}

TO_JSON_TPL(Object, VALUE.toJson().c_str())
