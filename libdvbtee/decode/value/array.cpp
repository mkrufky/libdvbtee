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
	vector.clear();
	fprintf(stderr, "%s\n", __func__);
}

Array::~Array()
{
	fprintf(stderr, "%s\n", __func__);
	if (vector.size()) for (KeyValueVector::iterator it = vector.begin(); it != vector.end(); ++it) {
		delete *it;
	}
	vector.clear();
}

Array::Array(const Array &obj)
{
	vector.clear();
	if (vector.size()) for (KeyValueVector::const_iterator it = obj.vector.begin(); it != obj.vector.end(); ++it) {
		vector.push_back(*it);
	}
	fprintf(stderr, "%s(copy) %lu\n", __func__, vector.size());
}

void Array::push(ValueBase *v)
{
	vector.push_back(v);
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

#if 0
namespace dvbtee {
namespace decode {

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
#endif
