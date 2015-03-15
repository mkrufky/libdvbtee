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

ValueBase::ValueBase(Type type, std::string name)
 : type(type)
 , name(name)
{
}

ValueBase::~ValueBase()
{
}

std::string ValueBase::toJson()
{
	std::stringstream s;

	switch(type) {
	case ValueBase::INTEGER:
		s << ((Value<ValueBase::INTEGER, int>*)this)->get();
		break;
	case ValueBase::UNSIGNEDSHORT:
		s << ((Value<ValueBase::UNSIGNEDSHORT, unsigned short>*)this)->get();
		break;
	case ValueBase::UNSIGNEDCHAR:
		s << (unsigned int)((Value<ValueBase::UNSIGNEDCHAR, unsigned char>*)this)->get();
		break;
	case ValueBase::STRING:
		s << "'" << ((Value<ValueBase::STRING, std::string>*)this)->get() << "'";
		break;
	case ValueBase::BOOLEAN:
		s << (((Value<ValueBase::BOOLEAN, bool>*)this)->get() ? "true" : "false");
		break;
	case ValueBase::DOUBLE:
		s << ((Value<ValueBase::DOUBLE, double>*)this)->get();
		break;
	case ValueBase::OBJECT:
		s << ((Value<ValueBase::OBJECT, Object>*)this)->get().toJson().c_str();
		break;
	case ValueBase::ARRAY:
		s << ((Value<ValueBase::ARRAY, Array>*)this)->get().toJson().c_str();
		break;
	}

	return s.str();
}
