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
#include "value-macros.h"

using namespace dvbtee::decode;

#define DBG 0

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
	clear();
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

const std::string Array::toJson()
{
	std::stringstream s;

	if (!vector.size()) return "[]";

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

const size_t Array::size()
{
	return vector.size();
}

ValueBase *Array::get(unsigned int idx)
{
	if (idx <= vector.size())
		return vector[idx];

	return NULL;
}

void Array::clear()
{
	for (KeyValueVector::iterator it = vector.begin(); it != vector.end(); ++it) {
		delete *it;
	}
	vector.clear();
}

void Array::push(ValueBase *val)
{
	     if (val->getType() == typeid(int))			push(((Value<int>*)val)->get());
	else if (val->getType() == typeid(unsigned short))	push(((Value<unsigned short>*)val)->get());
	else if (val->getType() == typeid(unsigned char))	push(((Value<unsigned char>*)val)->get());
	else if (val->getType() == typeid(std::string))		push(((Value<std::string>*)val)->get());
	else if (val->getType() == typeid(bool))		push(((Value<bool>*)val)->get());
	else if (val->getType() == typeid(double))		push(((Value<double>*)val)->get());
	else if (val->getType() == typeid(Object))		push(((Value<Object>*)val)->get());
	else if (val->getType() == typeid(Array))		push(((Value<Array>*)val)->get());
}

DEFINE_DEFAULT_GETTERS(Array, unsigned int)
