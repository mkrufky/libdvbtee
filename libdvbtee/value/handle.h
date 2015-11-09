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

//#include <string>

#include "value.h"
//#include "object.h"
//#include "array.h"

namespace valueobj {

//class Array;
//class Object;

class Handle {
public:
	Handle() : m_value(NULL) {}
	~Handle() { clear(); }

	Handle(const Handle& hdl) : m_value(hdl.m_value) { incRefCnt(); }
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

	Handle(ValueBase* v) : m_value(v) { incRefCnt(); }
	Handle(const ValueBase* v) : m_value((ValueBase*)v) { incRefCnt(); }

//	Handle(Object&);
//	Handle(Object*);
//	Handle(Array&);
//	Handle(Array*);
#if 1
	Handle& operator=(Handle& hdl)
	{
		set(hdl);
		return *this;
	}

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

	Handle& operator=(ValueBase* v)
	{
		set(v);
		return *this;
	}
//	Handle& operator=(Object&);
//	Handle& operator=(Object*);
//	Handle& operator=(Array&);
//	Handle& operator=(Array*);
#endif
#if 0
	template <typename T>
	const ValueBase* set(T& val)
	{
		std::string key("");
		return set((ValueBase*)new Value<T>(key, val));
	}
#endif
	const ValueBase* set(char* s)
	{
		std::string key("");
		std::string val(s);
		return set((ValueBase*)new Value<std::string>(key, val));
	}

	const ValueBase* set(const char* s)
	{
		std::string key("");
		std::string val(s);
		return set((ValueBase*)new Value<std::string>(key, val));
	}

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

	const ValueBase* set(Handle& hdl)
	{
		return set(hdl.m_value);
	}

	const ValueBase* set(const ValueBase* val)
	{
		return set((ValueBase*)val);
	}

	const ValueBase* set(ValueBase* val)
	{
		clear();
		m_value = val;
		incRefCnt();
		return m_value;
	}

	const ValueBase* get() const
	{
		return m_value;
	}

	template <typename T> const T& get() const
	{
		if (m_value) return ((Value<T>*)m_value)->get();
		//FIXME: return &valueobj::ReferencedValueUndefined::instance();
	}

	void clear()
	{
		decRefCnt();
		m_value = NULL;
	}

	size_t size() const { return (m_value == NULL) ? 0 : 1; }

	const std::string toJson() const
	{
		if (m_value) return m_value->toJson();
		return "";
	}

private:
	ValueBase* m_value;

	void incRefCnt() {
		if (m_value) ++(*m_value);
	}

	void decRefCnt()
	{
		if (m_value) {
		    if (!((--(*m_value)).getRefCnt())) {
			delete m_value;
		    }
		}
	}
};

}

#endif /* __HANDLE_H__ */
