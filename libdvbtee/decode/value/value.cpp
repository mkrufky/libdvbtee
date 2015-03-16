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
 : type(type)
 , name(name)
{
}

ValueBase::~ValueBase()
{
}

void ValueBase::badType(const std::type_info& typeRequested)
{
	fprintf(stderr, "Incorrect type requested for %s, requested %s, should be %s\n", name.length() ? name.c_str() : "[anonymous]", typeRequested.name(), typeid(type).name());
}

std::string ValueBase::toJson()
{
	std::stringstream s;

	     if (type == typeid(int))			s << ((Value<int>*)this)->get();
	else if (type == typeid(unsigned short))	s << ((Value<unsigned short>*)this)->get();
	else if (type == typeid(unsigned char))		s << ((Value<unsigned char>*)this)->get();
	else if (type == typeid(std::string))		s << "'" << ((Value<std::string>*)this)->get() << "'";
	else if (type == typeid(bool))			s << (((Value<bool>*)this)->get() ? "true" : "false");
	else if (type == typeid(double))		s << ((Value<double>*)this)->get();
	else if (type == typeid(Object))		s << ((Value<Object>*)this)->get().toJson().c_str();
	else if (type == typeid(Array))			s << ((Value<Array>*)this)->get().toJson().c_str();

	return s.str();
}
