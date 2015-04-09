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

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <map>
#include <string>
#include <vector>

#include "value.h"
#include "array.h"

namespace valueobj {

typedef std::map<std::string, ValueBase*> KeyValueMap;

class Array;

class Object {
public:
	Object();
	~Object();

	Object(const Object&);

	template <typename T>
	const ValueBase* set(std::string key, T val);

	const ValueBase* set(std::string key,       char* val);
	const ValueBase* set(std::string key, const char* val);
	const ValueBase* set(std::string key, std::string& val);
	const ValueBase* set(std::string key, Array& val);
	const ValueBase* set(std::string key, Object& val);
	const ValueBase* set(std::string key, Array* val);
	const ValueBase* set(std::string key, Object* val);

	template <typename T>
	const ValueBase* set(int key, T val);

	const ValueBase* set(ValueBase*);

	void unSet(std::string key);
	void unSet(int key);

	void clear();

	const ValueBase* get(std::string key) const;
	const ValueBase* get(int key) const;

	template <typename T> const T& get(std::string key) const;

	template <typename T>
	const T& get(int key) const;

	const std::string toJson() const;

private:
	KeyValueMap map;

	template <typename T>
	const ValueBase* setByRef(std::string& key, T& val);

	template <typename T>
	const T& get(std::string& key, T& def) const;

	const std::string intToStr(int) const;
};

}

#endif /* __OBJECT_H__ */
