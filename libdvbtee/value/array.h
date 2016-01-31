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

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <map>
#include <string>
#include <vector>

#include "value.h"
#include "object.h"
#include "handle.h"

namespace valueobj {

typedef std::vector<Handle> KeyValueVector;

class Object;

class Array {
public:
	Array(std::string idx = "");
	~Array();

	Array(const Array&);

	Handle& push(Handle hdl);

	Handle& push(ValueBase*);

	Handle& push(Object&);
	Handle& push(Object*);

	template <typename T>
	Handle& push(T val)
	{
		return push(Handle(val, ""));
	}

	bool set(std::string key, Handle hdl);

	template <typename T>
	bool set(std::string key, T val)
	{
		return set(key, Handle(val, key));
	}

	template <typename T>
	bool set(int key, T val)
	{
		return set(intToStr(key), val);
	}


	Handle& getByName(std::string idx) const;
	Handle& getByName(unsigned int idx) const;

	Handle& get(unsigned int idx) const;

	template <typename T> const T& get(unsigned int idx) const;

	void clear();

	size_t size() const;

	const std::string& getIndex() const;

	const std::string toJson() const;

private:
	KeyValueVector vector;
	std::map<std::string, Handle*> indices;
	std::string idxField;

	Handle& pushObject(Object& val, std::string idx);

	template <typename T>
	const T& get(unsigned int &idx, T& def) const;

	void updateIndex(std::string, Handle&);
	std::string& assignIndex(Object&, std::string&);

	const std::string intToStr(int) const;
};

}

#endif /* __ARRAY_H__ */
