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
#include "value.h"
#include "array.h"
#include "object.h"

using namespace dvbtee::decode;

ValueBase::ValueBase(const std::type_info& type, std::string name)
 : m_type(type)
 , m_name(name)
{
}

ValueBase::~ValueBase()
{
}

const std::type_info& ValueBase::getType()
{
	return m_type;
}

const std::string& ValueBase::getName()
{
	return m_name;
}

bool ValueBase::checkType(const std::type_info& typeRequested)
{
	bool ret = (typeRequested == m_type);
	if (!ret) fprintf(stderr, "Incorrect type requested for %s, requested %s, should be %s\n", m_name.length() ? m_name.c_str() : "[anonymous]", typeRequested.name(), typeid(m_type).name());
	return ret;
}

const std::string ValueBase::toJson()
{
	std::stringstream s;

	     if (m_type == typeid(int))			s << ((Value<int>*)this)->get();
	else if (m_type == typeid(long))		s << ((Value<long>*)this)->get();
	else if (m_type == typeid(unsigned short))	s << ((Value<unsigned short>*)this)->get();
	else if (m_type == typeid(unsigned char))	s << (unsigned int)((Value<unsigned char>*)this)->get();
	else if (m_type == typeid(std::string))		s << "'" << ((Value<std::string>*)this)->get() << "'";
	else if (m_type == typeid(bool))		s << (((Value<bool>*)this)->get() ? "true" : "false");
	else if (m_type == typeid(double))		s << ((Value<double>*)this)->get();
	else if (m_type == typeid(Object))		s << ((Value<Object>*)this)->get().toJson().c_str();
	else if (m_type == typeid(Array))		s << ((Value<Array>*)this)->get().toJson().c_str();
	else if (m_type == typeid(time_t))		s << (unsigned long)((Value<time_t>*)this)->get();
	else if (m_type == typeid(size_t))		s << (unsigned long)((Value<time_t>*)this)->get();

	if (s.str().length())
		return s.str();

	return "undefined";
}
