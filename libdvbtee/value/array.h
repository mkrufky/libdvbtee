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

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <map>
#include <string>
#include <vector>

#include "value.h"
#include "object.h"

namespace valueobj {

typedef std::vector<ValueBase*> KeyValueVector;

class Object;

class Array {
public:
	Array(std::string idx = "");
	~Array();

	Array(const Array&);

	const ValueBase* push(ValueBase*);

	const ValueBase* push(Object&);
	const ValueBase* push(Object*);

	template <typename T>
	const ValueBase* push(T val);

	template <typename T>
	const ValueBase* set(std::string key, T val);

	template <typename T>
	const ValueBase* set(int key, T val);

	const ValueBase* getByName(std::string idx) const;
	const ValueBase* getByName(unsigned int idx) const;

	const ValueBase* get(unsigned int idx) const;

	template <typename T> const T& get(unsigned int idx) const;

	void clear();

	size_t size() const;

	const std::string& getIndex() const;

	const std::string toJson() const;

private:
	KeyValueVector vector;
	std::map<std::string, const ValueBase*> indices;
	std::string idxField;

	template <typename T>
	const ValueBase* pushByRef(T& val, std::string idx);

	const ValueBase* pushObject(Object& val, std::string idx);

	template <typename T>
	const ValueBase* push(T val, std::string idx);

#define USING_INLINE_PUSH
#ifdef USING_INLINE_PUSH
	inline const ValueBase* push(      char* val, std::string idx)	{ return push<std::string>(std::string(val), idx); }
	inline const ValueBase* push(const char* val, std::string idx)	{ return push<std::string>(std::string(val), idx); }
	inline const ValueBase* push(std::string& val, std::string idx)	{ return pushByRef<std::string>(val, idx); }
	inline const ValueBase* push(Array& val, std::string idx)	{ return pushByRef<Array>(val, idx); }
	inline const ValueBase* push(Array* val, std::string idx)	{ return pushByRef<Array>(*val, idx); }
#else
	const ValueBase* push(      char* val, std::string idx);
	const ValueBase* push(const char* val, std::string idx);
	const ValueBase* push(std::string& val, std::string idx);
	const ValueBase* push(Array& val, std::string idx);
	const ValueBase* push(Array* val, std::string idx);
#endif

	template <typename T>
	const T& get(unsigned int &idx, T& def) const;

	void updateIndex(std::string, const ValueBase*);
	std::string& assignIndex(Object&, std::string&);

	const std::string intToStr(int) const;
};

}

#endif /* __ARRAY_H__ */
