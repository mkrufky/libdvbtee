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

Object::Object()
{
	fprintf(stderr, "%s\n", __func__);
}

Object::~Object()
{
	fprintf(stderr, "%s\n", __func__);
	for (KeyValueMap::iterator it = map.begin(); it != map.end(); ++it) {
		//Value<TYPE, T> *value = (Value<TYPE, T>*)map[key];
		delete it->second;
	}
}

Object::Object(const Object &obj)
{
	for (KeyValueMap::const_iterator it = obj.map.begin(); it != obj.map.end(); ++it) {
		set(it->second);
	}

	fprintf(stderr, "%s(copy) %lu\n", __func__, map.size());
}

ValueBase *Object::get(std::string key)
{
	if (map.count(key))
		return map[key];

	return NULL;
}

std::string Object::toJson()
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

void Object::badType(ValueBase::Type typeRequested, ValueBase *val)
{
	fprintf(stderr, "Incorrect type requested for %s, requested %d, should be %d\n", val->name.c_str(), typeRequested, (int)val->type);
}

namespace dvbtee {
namespace decode {

template <>
int& Object::get<int>(std::string key)
{
	int def = 0;
	return get<ValueBase::INTEGER, int>(key, def);
}

template <>
unsigned short& Object::get<unsigned short>(std::string key)
{
	unsigned short def = 0;
	return get<ValueBase::UNSIGNEDSHORT, unsigned short>(key, def);
}

template <>
unsigned char& Object::get<unsigned char>(std::string key)
{
	unsigned char def = 0;
	return get<ValueBase::UNSIGNEDCHAR, unsigned char>(key, def);
}

template <>
std::string& Object::get<std::string>(std::string key)
{
	std::string def = "";
	return get<ValueBase::STRING, std::string>(key, def);
}

template <>
bool& Object::get<bool>(std::string key)
{
	bool def = false;
	return get<ValueBase::BOOLEAN, bool>(key, def);
}

template <>
double& Object::get<double>(std::string key)
{
	double def = 0.0;
	return get<ValueBase::DOUBLE, double>(key, def);
}

template <>
Object& Object::get<Object>(std::string key)
{
	Object def;
	return get<ValueBase::OBJECT, Object>(key, def);
}

#if 1
template <>
Array& Object::get<Array>(std::string key)
{
	Array def;
	return get<ValueBase::ARRAY, Array>(key, def);
}
#endif


template <>
void Object::set<int>(std::string key, int val)
{
	return set<ValueBase::INTEGER, int>(key, val);
}

template <>
void Object::set<unsigned short>(std::string key, unsigned short val)
{
	return set<ValueBase::UNSIGNEDSHORT, unsigned short>(key, val);
}

template <>
void Object::set<unsigned char>(std::string key, unsigned char val)
{
	return set<ValueBase::UNSIGNEDCHAR, unsigned char>(key, val);
}

template <>
void Object::set<std::string>(std::string key, std::string val)
{
	return set<ValueBase::STRING, std::string>(key, val);
}

template <>
void Object::set<bool>(std::string key, bool val)
{
	return set<ValueBase::BOOLEAN, bool>(key, val);
}

template <>
void Object::set<double>(std::string key, double val)
{
	return set<ValueBase::DOUBLE, double>(key, val);
}

#if 0
template <>
void Object::set<Object>(std::string key, Object val)
{
	return set<ValueBase::OBJECT, Object>(key, val);
}

#if 1
template <>
void Object::set<Array>(std::string key, Array val)
{
	return set<ValueBase::ARRAY, Array>(key, val);
}
#endif
#endif

}}

void Object::set(ValueBase *val)
{
	switch(val->type) {
	case ValueBase::INTEGER:
		set(val->name, ((Value<ValueBase::INTEGER, int>*)val)->get());
		break;
	case ValueBase::UNSIGNEDSHORT:
		set(val->name, ((Value<ValueBase::UNSIGNEDSHORT, unsigned short>*)val)->get());
		break;
	case ValueBase::UNSIGNEDCHAR:
		set(val->name, ((Value<ValueBase::UNSIGNEDCHAR, unsigned char>*)val)->get());
		break;
	case ValueBase::STRING:
		set(val->name, ((Value<ValueBase::STRING, std::string>*)val)->get());
		break;
	case ValueBase::BOOLEAN:
		set(val->name, ((Value<ValueBase::BOOLEAN, bool>*)val)->get());
		break;
	case ValueBase::DOUBLE:
		set(val->name, ((Value<ValueBase::DOUBLE, double>*)val)->get());
		break;
	case ValueBase::OBJECT:
		set(val->name, ((Value<ValueBase::OBJECT, Object>*)val)->get());
		break;
	case ValueBase::ARRAY:
		set(val->name, ((Value<ValueBase::ARRAY, Array>*)val)->get());
		break;
	}
}
