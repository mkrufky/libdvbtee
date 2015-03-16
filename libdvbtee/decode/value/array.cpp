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
#include "array.h"

using namespace dvbtee::decode;

Array::Array()
{
#if DBG
	fprintf(stderr, "%s\n", __func__);
#endif
}

Array::~Array()
{
#if DBG
	fprintf(stderr, "%s\n", __func__);
#endif
	for (KeyValueVector::iterator it = vector.begin(); it != vector.end(); ++it) {
		delete *it;
	}
}

Array::Array(const Array &obj)
{
	for (KeyValueVector::const_iterator it = obj.vector.begin(); it != obj.vector.end(); ++it) {
		push(*it);
	}
#if DBG
	fprintf(stderr, "%s(copy) %lu\n", __func__, vector.size());
#endif
}

std::string Array::toJson()
{
	std::stringstream s;
	int count = 0;

	s << "[ ";

	for (KeyValueVector::const_iterator it = vector.begin(); it != vector.end(); ++it) {
		if (count) s << ", ";

		s << (*it)->toJson();

		count++;
	}
	s << " ]";

	return s.str();

}

size_t Array::size()
{
	return vector.size();
}

ValueBase *Array::get(unsigned int idx)
{
	if (idx <= vector.size())
		return vector[idx];

	return NULL;
}

namespace dvbtee {
namespace decode {

template <>
int& Array::get<int>(unsigned int idx)
{
	int def = 0;
	return get<ValueBase::INTEGER, int>(idx, def);
}

template <>
unsigned short& Array::get<unsigned short>(unsigned int idx)
{
	unsigned short def = 0;
	return get<ValueBase::UNSIGNEDSHORT, unsigned short>(idx, def);
}

template <>
unsigned char& Array::get<unsigned char>(unsigned int idx)
{
	unsigned char def = 0;
	return get<ValueBase::UNSIGNEDCHAR, unsigned char>(idx, def);
}

template <>
std::string& Array::get<std::string>(unsigned int idx)
{
	std::string def = "";
	return get<ValueBase::STRING, std::string>(idx, def);
}

template <>
bool& Array::get<bool>(unsigned int idx)
{
	bool def = false;
	return get<ValueBase::BOOLEAN, bool>(idx, def);
}

template <>
double& Array::get<double>(unsigned int idx)
{
	double def = 0.0;
	return get<ValueBase::DOUBLE, double>(idx, def);
}

template <>
Object& Array::get<Object>(unsigned int idx)
{
	Object def;
	return get<ValueBase::OBJECT, Object>(idx, def);
}

#if 1
template <>
Array& Array::get<Array>(unsigned int idx)
{
	Array def;
	return get<ValueBase::ARRAY, Array>(idx, def);
}
#endif


#if 0
template <>
Array& Object::get<Array>(std::string key)
{
	Array def;
	return get<ValueBase::ARRAY, Array>(key, def);
}

#if 0
template <>
void Object::set<Array>(std::string key, Array val)
{
	return set<ValueBase::ARRAY, Array>(key, val);
}
#endif
#endif

template <>
void Array::push<int>(int val)
{
	push<ValueBase::INTEGER, int>(val);
}

template <>
void Array::push<unsigned short>(unsigned short val)
{
	push<ValueBase::UNSIGNEDSHORT, unsigned short>(val);
}

template <>
void Array::push<unsigned char>(unsigned char val)
{
	push<ValueBase::UNSIGNEDCHAR, unsigned char>(val);
}

template <>
void Array::push<std::string>(std::string val)
{
	push<ValueBase::STRING, std::string>(val);
}

template <>
void Array::push<bool>(bool val)
{
	push<ValueBase::BOOLEAN, bool>(val);
}

template <>
void Array::push<double>(double val)
{
	push<ValueBase::DOUBLE, double>(val);
}

}}

void Array::push(ValueBase *val)
{
	switch(val->type) {
	case ValueBase::INTEGER:
		push(((Value<ValueBase::INTEGER, int>*)val)->get());
		break;
	case ValueBase::UNSIGNEDSHORT:
		push(((Value<ValueBase::UNSIGNEDSHORT, unsigned short>*)val)->get());
		break;
	case ValueBase::UNSIGNEDCHAR:
		push(((Value<ValueBase::UNSIGNEDCHAR, unsigned char>*)val)->get());
		break;
	case ValueBase::STRING:
		push(((Value<ValueBase::STRING, std::string>*)val)->get());
		break;
	case ValueBase::BOOLEAN:
		push(((Value<ValueBase::BOOLEAN, bool>*)val)->get());
		break;
	case ValueBase::DOUBLE:
		push(((Value<ValueBase::DOUBLE, double>*)val)->get());
		break;
	case ValueBase::OBJECT:
		push(((Value<ValueBase::OBJECT, Object>*)val)->get());
		break;
	case ValueBase::ARRAY:
		push(((Value<ValueBase::ARRAY, Array>*)val)->get());
		break;
	}
}
