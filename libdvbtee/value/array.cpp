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

using namespace valueobj;

namespace valueobj {

template <typename T>
const ValueBase* Array::push(T val)
{
	return push<T>(val, "");
}

template <typename T>
const ValueBase* Array::set(std::string key, T val)
{
	if (!key.length()) return NULL;
	const ValueBase* v = push<T>(val, key);
	if (v) updateIndex(key, v);
	return v;
}

template <typename T>
const ValueBase* Array::set(int key, T val)
{
	return set<T>(intToStr(key), val);
}

template <typename T>
const ValueBase* Array::pushByRef(T& val, std::string idx)
{
	Value<T> *v = new Value<T>(idx, val);
	vector.push_back(v);
	++(*v); // increment refcount
	return v;
}

template <typename T>
const ValueBase* Array::push(T val, std::string idx)
{
	return pushByRef<T>(val, idx);
}


template <typename T>
const T& Array::get(unsigned int &idx, T& def) const
{
	if (idx <= vector.size()) {
		Value<T> *val = (Value<T>*)vector[idx];
		if (val->checkType(typeid(T)))
			return val->get();
	}
	return def;
}
}

#define IMPL_ARRAY_TMPL(T) \
template const ValueBase* Array::push(T val); \
template const ValueBase* Array::set(std::string key, T val); \
template const ValueBase* Array::set(int key, T val); \
template const ValueBase* Array::pushByRef(T& val, std::string idx); \
template const ValueBase* Array::push(T val, std::string idx); \
template const T& Array::get(unsigned int &idx, T& def) const

#if 1 // we only need Array::push(unsigned int) but implementing them all for the sake of completeness
IMPL_ARRAY_TMPL(unsigned int);
#else
template const ValueBase* Array::push(unsigned int val);
#endif

#define DBG 0

static ReferencedValueUndefined& valueUndefined = ReferencedValueUndefined::instance();

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
		ValueBase *v = *it;
		push(v);
		const std::string& n = v->getName();
		if (n.length()) updateIndex(n, v);
	}
#if DBG
	fprintf(stderr, "%s(copy) %lu\n", __func__, vector.size());
#endif
}

const std::string Array::toJson() const
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

size_t Array::size() const
{
	return vector.size();
}

const std::string &Array::getIndex() const
{
	return idxField;
}

const ValueBase* Array::get(unsigned int idx) const
{
	if (idx <= vector.size())
		return vector[idx];

	return &valueUndefined;
}

void Array::updateIndex(std::string key, const ValueBase *val)
{
	if (key.length()) indices[key] = val;
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

const ValueBase* Array::getByName(std::string idx) const
{
	std::map<std::string, const ValueBase*>::const_iterator it = indices.find(idx);
	if (it == indices.end())
		return &valueUndefined;

	return it->second;
}

const ValueBase* Array::getByName(unsigned int idx) const
{
	return getByName(intToStr(idx));
}

void Array::clear()
{
	for (KeyValueVector::iterator it = vector.begin(); it != vector.end(); ++it)
	{
		// decrement refcount. if refcount becomes zero, delete
		if (0 == (--(**it)).getRefCnt()) delete *it;
	}
	vector.clear();
	indices.clear();
}

const std::string Array::intToStr(int i) const
{
	std::stringstream s;
	s << i;
	return s.str();
}

const ValueBase* Array::pushObject(Object &val, std::string idx)
{
	bool extractIndex = (!idx.length());

	if (extractIndex) assignIndex(val, idx);

	const ValueBase *v = pushByRef<Object>(val, idx);

	if (extractIndex) updateIndex(idx, v);

	return v;
}

#ifndef USING_INLINE_PUSH
const ValueBase *Array::push(char *val, std::string idx)
{
	return push<std::string>(std::string(val), idx);
}

const ValueBase *Array::push(const char *val, std::string idx)
{
	return push<std::string>(std::string(val), idx);
}

const ValueBase *Array::push(std::string &val, std::string idx)
{
	return pushByRef<std::string>(val, idx);
}

const ValueBase *Array::push(Array &val, std::string idx)
{
	return pushByRef<Array>(val, idx);
}

const ValueBase *Array::push(Array *val, std::string idx)
{
	return pushByRef<Array>(*val, idx);
}
#endif

const ValueBase* Array::push(Object &o)
{
	return pushObject(o, "");
}

const ValueBase *Array::push(Object *o)
{
	return push(*o);
}

const ValueBase* Array::push(ValueBase *val)
{
	vector.push_back(val);
	++(*val);
	return val;
}

DEFINE_DEFAULT_GETTERS(Array, unsigned int)
TO_JSON_TPL(Array, VALUE.toJson().c_str())
