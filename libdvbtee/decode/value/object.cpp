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
	for (KeyValueMap::iterator it = map.begin(); it != map.end(); ++it) {
		delete it->second;
	}
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

const std::string Object::toJson()
{
	std::stringstream s;
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

namespace dvbtee {
namespace decode {

template <>
int& Object::get<int>(std::string key)
{
	int def = 0;
	return get<int>(key, def);
}

template <>
unsigned short& Object::get<unsigned short>(std::string key)
{
	unsigned short def = 0;
	return get<unsigned short>(key, def);
}

template <>
unsigned char& Object::get<unsigned char>(std::string key)
{
	unsigned char def = 0;
	return get<unsigned char>(key, def);
}

template <>
std::string& Object::get<std::string>(std::string key)
{
	std::string def = "";
	return get<std::string>(key, def);
}

template <>
bool& Object::get<bool>(std::string key)
{
	bool def = false;
	return get<bool>(key, def);
}

template <>
double& Object::get<double>(std::string key)
{
	double def = 0.0;
	return get<double>(key, def);
}

template <>
Object& Object::get<Object>(std::string key)
{
	Object def;
	return get<Object>(key, def);
}

template <>
Array& Object::get<Array>(std::string key)
{
	Array def;
	return get<Array>(key, def);
}

}}

void Object::set(ValueBase *val)
{
	     if (val->type == typeid(int))		set(val->name, ((Value<int>*)val)->get());
	else if (val->type == typeid(unsigned short))	set(val->name, ((Value<unsigned short>*)val)->get());
	else if (val->type == typeid(unsigned char))	set(val->name, ((Value<unsigned char>*)val)->get());
	else if (val->type == typeid(std::string))	set(val->name, ((Value<std::string>*)val)->get());
	else if (val->type == typeid(bool))		set(val->name, ((Value<bool>*)val)->get());
	else if (val->type == typeid(double))		set(val->name, ((Value<double>*)val)->get());
	else if (val->type == typeid(Object))		set(val->name, ((Value<Object>*)val)->get());
	else if (val->type == typeid(Array))		set(val->name, ((Value<Array>*)val)->get());
}
