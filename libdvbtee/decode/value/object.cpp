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

using namespace dvbtee::decode;

#define DBG 0

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

ValueBase *Object::get(std::string key)
{
	if (map.count(key))
		return map[key];

	return NULL;
}

void Object::unSet(std::string key)
{
	if (map.count(key)) {
		delete map[key];
		map.erase(key);
	}
}

void Object::clear()
{
	for (KeyValueMap::iterator it = map.begin(); it != map.end(); ++it) {
		delete it->second;
	}
	map.clear();
}

const std::string Object::toJson()
{
	std::stringstream s;

	if (!map.size()) return "{}";

	int count = 0;

	s << "{ ";
	for (KeyValueMap::iterator it = map.begin(); it != map.end(); ++it) {
		ValueBase *val = it->second;

		if (count) s << ", ";
		s << "'" << it->first << "': ";

		s << val->toJson();

		count++;
	}
	s << " }";

	return s.str();
}

std::string Object::intToStr(int i)
{
	std::stringstream s;
	s << i;
	return s.str();
}

void Object::set(ValueBase *val)
{
	     if (val->getType() == typeid(int))			set(val->getName(), ((Value<int>*)val)->get());
	else if (val->getType() == typeid(unsigned short))	set(val->getName(), ((Value<unsigned short>*)val)->get());
	else if (val->getType() == typeid(unsigned char))	set(val->getName(), ((Value<unsigned char>*)val)->get());
	else if (val->getType() == typeid(std::string))		set(val->getName(), ((Value<std::string>*)val)->get());
	else if (val->getType() == typeid(bool))		set(val->getName(), ((Value<bool>*)val)->get());
	else if (val->getType() == typeid(double))		set(val->getName(), ((Value<double>*)val)->get());
	else if (val->getType() == typeid(Object))		set(val->getName(), ((Value<Object>*)val)->get());
	else if (val->getType() == typeid(Array))		set(val->getName(), ((Value<Array>*)val)->get());
}

DEFINE_DEFAULT_GETTERS(Object, std::string)
