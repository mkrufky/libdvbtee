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

#ifndef __HANDLE_H__
#define __HANDLE_H__

#include <string>
#include <stddef.h>

#include "value.h"
//#include "object.h"
//#include "array.h"

namespace valueobj {

//class Array;
//class Object;

class Handle {
public:
	Handle();
	~Handle();

	Handle(const Handle& hdl);
#if 0
	template <typename T>
	Handle(T& o) : m_value(NULL) { set(o); }
#endif
	template <typename T>
	Handle(T o) : m_value(NULL) { set(o); }

//	Handle(char* s) : m_value(NULL) { set(std::string(s)); }
//	Handle(const char *s) : m_value(NULL) { set(std::string(s)); }

//	Handle(const Object& o);
//	Handle(const Array& o);

	template <typename T>
	Handle(Value<T>& v) : m_value(&v) { incRefCnt(); }

	Handle(const ValueBase* v);
	Handle(ValueBase* v);

//	Handle(Object&);
//	Handle(Object*);
//	Handle(Array&);
//	Handle(Array*);

	Handle& operator=(Handle& hdl);

	template <typename T>
	Handle& operator=(T& v)
	{
		set(v);
		return *this;
	}

	template <typename T>
	Handle& operator=(T v)
	{
		set(v);
		return *this;
	}

	template <typename T>
	Handle& operator=(Value<T>& v)
	{
		set(v);
		return *this;
	}

	Handle& operator=(ValueBase* v);
//	Handle& operator=(Object&);
//	Handle& operator=(Object*);
//	Handle& operator=(Array&);
//	Handle& operator=(Array*);

#if 0
	template <typename T>
	const ValueBase* set(T& val)
	{
		std::string key("");
		return set((ValueBase*)new Value<T>(key, val));
	}
#endif
	const ValueBase* set(char* s);

	const ValueBase* set(const char* s);

	template <typename T>
	const ValueBase* set(T val)
	{
		std::string key("");
		return set((ValueBase*)new Value<T>(key, val));
	}

	template <typename T>
	const ValueBase* set(Value<T>& val)
	{
		return set((ValueBase*)&val);
	}

	const ValueBase* set(Handle& hdl);

	const ValueBase* set(const ValueBase* val);

	const ValueBase* set(ValueBase* val);

	const ValueBase* get() const;

	template <typename T> const T& get() const
	{
		if (m_value) return ((Value<T>*)m_value)->get();
		//FIXME: return &valueobj::ReferencedValueUndefined::instance();
	}

	template <typename T>
	operator T() const { return get<T>(); }

	operator ValueBase*() const;

	void clear();

	size_t size() const;

	const std::string toJson() const;

private:
	ValueBase* m_value;

	void incRefCnt();
	void decRefCnt();
};

}

#endif /* __HANDLE_H__ */
