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
#include "handle.h"

namespace valueobj {

typedef std::vector<ValueBase*> KeyValueVector;

class Object;

class Array {
public:
	Array(std::string idx = "");
	~Array();

	Array(const Array&);

	const ValueBase* push(Handle hdl);
	const ValueBase* set(Handle hdl);

	const ValueBase* push(ValueBase*);

	const ValueBase* push(Object&);
	const ValueBase* push(Object*);

	template <typename T>
	const ValueBase* push(T val)
	{
		return push(Handle(val, ""));
	}

	template <typename T>
	const ValueBase* set(std::string key, T val);

	const ValueBase* set(std::string key,       char* val);
	const ValueBase* set(std::string key, const char* val);

	template <typename T>
	const ValueBase* set(int key, T val);

	const ValueBase* set(int key,       char* val);
	const ValueBase* set(int key, const char* val);

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

	const ValueBase* pushObject(Object& val, std::string idx);

	template <typename T>
	const T& get(unsigned int &idx, T& def) const;

	void updateIndex(std::string, const ValueBase*);
	std::string& assignIndex(Object&, std::string&);

	const std::string intToStr(int) const;
};

}

#endif /* __ARRAY_H__ */
