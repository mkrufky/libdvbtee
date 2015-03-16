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
	return get<int>(idx, def);
}

template <>
unsigned short& Array::get<unsigned short>(unsigned int idx)
{
	unsigned short def = 0;
	return get<unsigned short>(idx, def);
}

template <>
unsigned char& Array::get<unsigned char>(unsigned int idx)
{
	unsigned char def = 0;
	return get<unsigned char>(idx, def);
}

template <>
std::string& Array::get<std::string>(unsigned int idx)
{
	std::string def = "";
	return get<std::string>(idx, def);
}

template <>
bool& Array::get<bool>(unsigned int idx)
{
	bool def = false;
	return get<bool>(idx, def);
}

template <>
double& Array::get<double>(unsigned int idx)
{
	double def = 0.0;
	return get<double>(idx, def);
}

template <>
Object& Array::get<Object>(unsigned int idx)
{
	Object def;
	return get<Object>(idx, def);
}

template <>
Array& Array::get<Array>(unsigned int idx)
{
	Array def;
	return get<Array>(idx, def);
}

}}

void Array::push(ValueBase *val)
{
	     if (val->type == typeid(int))		push(((Value<int>*)val)->get());
	else if (val->type == typeid(unsigned short))	push(((Value<unsigned short>*)val)->get());
	else if (val->type == typeid(unsigned char))	push(((Value<unsigned char>*)val)->get());
	else if (val->type == typeid(std::string))	push(((Value<std::string>*)val)->get());
	else if (val->type == typeid(bool))		push(((Value<bool>*)val)->get());
	else if (val->type == typeid(double))		push(((Value<double>*)val)->get());
	else if (val->type == typeid(Object))		push(((Value<Object>*)val)->get());
	else if (val->type == typeid(Array))		push(((Value<Array>*)val)->get());
}
