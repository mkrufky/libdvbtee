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

#define DBG 0

#if DBG
#include <stdio.h>
#endif
#include <sstream>
#include "object.h"
#include "value-macros.h"

using namespace valueobj;

DEFINE_DEFAULT_GETTERS(Object, std::string)

namespace valueobj {

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


static ReferencedValueUndefined& valueUndefined = ReferencedValueUndefined::instance();
static Handle valueUndefinedHdl = Handle((ValueBase*)&valueUndefined);

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
		set(it->first, it->second);
	}

#if DBG
	fprintf(stderr, "%s(copy) %lu\n", __func__, map.size());
#endif
}

Handle& Object::setByRef(std::string& key, Handle& hdl)
{
	map[key] = hdl;
	return hdl;
}

Handle& Object::get(std::string key) const
{
	KeyValueMap::const_iterator it = map.find(key);
	if (it != map.end()) {
		return (Handle&)it->second;
	}

	return valueUndefinedHdl;
}

Handle& Object::get(int key) const
{
	return get(intToStr(key));
}

void Object::unSet(std::string key)
{
	if (map.count(key)) {
		map.erase(key);
	}
}

void Object::unSet(int key)
{
	unSet(intToStr(key));
}

void Object::clear()
{
	map.clear();
}

const std::string Object::toJson() const
{
	std::stringstream s;

	if (!map.size()) return "{}";

	s << "{ ";
	for (KeyValueMap::const_iterator it = map.begin(); it != map.end(); ++it) {
		if (it != map.begin()) s << ", ";

		s << "'" << it->first << "': " << it->second.toJson();
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

Handle& Object::set(std::string key, Handle hdl)
{
	return setByRef(key, hdl);
}

Handle& Object::set(int key, Handle hdl)
{
	return set(intToStr(key), hdl);
}

// deprecated:
Handle& Object::set(ValueBase *val)
{
	return set(val->getName(), val);
}

Handle& Object::set(std::string key, ValueBase *val)
{
	return set(key, Handle(val));
}

TO_JSON_TPL(Object, VALUE.toJson().c_str())
