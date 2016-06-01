/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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
#include "array.h"
#include "value-macros.h"

using namespace valueobj;

namespace valueobj {

template <typename T>
const T& Array::get(unsigned int &idx, T& def) const
{
	if (idx <= vector.size()) {
		Value<T> *val = vector.at(idx);
		if (val->checkType(typeid(T)))
			return val->get();
	}
	return def;
}
}

#define IMPL_ARRAY_TMPL(T) \
template Handle& Array::push(T val); \
template bool Array::set(std::string key, T val); \
template bool Array::set(int key, T val); \
template const T& Array::get(unsigned int &idx, T& def) const

IMPL_ARRAY_TMPL(int);
IMPL_ARRAY_TMPL(long);
IMPL_ARRAY_TMPL(long long);
IMPL_ARRAY_TMPL(short);
IMPL_ARRAY_TMPL(char);
IMPL_ARRAY_TMPL(unsigned int);
IMPL_ARRAY_TMPL(unsigned long);
IMPL_ARRAY_TMPL(unsigned long long);
IMPL_ARRAY_TMPL(unsigned short);
IMPL_ARRAY_TMPL(unsigned char);
IMPL_ARRAY_TMPL(std::string);
IMPL_ARRAY_TMPL(bool);
IMPL_ARRAY_TMPL(double);
IMPL_ARRAY_TMPL(Array);
IMPL_ARRAY_TMPL(Object);

static ReferencedValueUndefined& valueUndefined = ReferencedValueUndefined::instance();
static Handle valueUndefinedHdl = Handle((ValueBase*)&valueUndefined);

Array::Array(std::string idx)
 : idxField(idx)
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
		Handle& hdl = push(*it);
		const std::string& n = hdl.get()->getName();
		if (n.length()) updateIndex(n, hdl);
	}
#if DBG
	fprintf(stderr, "%s(copy) %lu\n", __func__, vector.size());
#endif
}

Handle& Array::push(Handle hdl)
{
	vector.push_back(hdl);
	return vector.back();
}

bool Array::set(std::string key, Handle hdl)
{
	if (!key.length()) return false;

	Handle& h = push(hdl);
	updateIndex(key, h);
	return true;
}

const std::string Array::toJson() const
{
	std::stringstream s;

	if (!vector.size()) return "[]";

	s << "[ ";

	for (KeyValueVector::const_iterator it = vector.begin(); it != vector.end(); ++it) {
		if (it != vector.begin()) s << ", ";

		s << it->toJson();
	}
	s << " ]";

	return s.str();

}

size_t Array::size() const
{
	return vector.size();
}

const std::string &Array::getIndex() const
{
	return idxField;
}

Handle& Array::get(unsigned int idx) const
{
	if (idx <= vector.size())
		return (Handle&)vector.at(idx);

	return valueUndefinedHdl;
}

void Array::updateIndex(std::string key, Handle& val)
{
	if (key.length()) indices[key] = &val;
}

std::string &Array::assignIndex(Object &obj, std::string &index)
{
	if (idxField.length()) {
		const ValueBase *val = obj.get(idxField);
		index = (typeid(std::string) == val->getType()) ?
			obj.get<std::string>(idxField) : val->toJson();
	}

	return index;
}

Handle& Array::getByName(std::string idx) const
{
	std::map<std::string, Handle*>::const_iterator it = indices.find(idx);
	if (it == indices.end())
		return valueUndefinedHdl;

	return *(it->second);
}

Handle& Array::getByName(unsigned int idx) const
{
	return getByName(intToStr(idx));
}

void Array::clear()
{
	vector.clear();
	indices.clear();
}

const std::string Array::intToStr(int i) const
{
	std::stringstream s;
	s << i;
	return s.str();
}

Handle& Array::pushObject(Object &val, std::string idx)
{
	bool extractIndex = (!idx.length());

	if (extractIndex) assignIndex(val, idx);

	Handle& v = push(Handle(val, idx));

	if (extractIndex) updateIndex(idx, v);

	return v;
}

Handle& Array::push(Object &o)
{
	return pushObject(o, "");
}

Handle& Array::push(Object *o)
{
	return push(*o);
}

Handle& Array::push(ValueBase *val)
{
	return push(Handle(val));
}

DEFINE_DEFAULT_GETTERS(Array, unsigned int)
TO_JSON_TPL(Array, VALUE.toJson().c_str())
