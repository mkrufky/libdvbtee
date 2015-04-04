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

#include "value.h"
#include "array.h"
#include "object.h"
#include "value-macros.h"

using namespace dvbtee::decode;

ValueBase::ValueBase(const std::type_info& type, std::string name)
 : m_type(type)
 , m_name(name)
 , m_refcnt(0)
{
}

ValueBase &ValueBase::operator++()
{
	m_refcnt++;
	return *this;
}

ValueBase &ValueBase::operator--()
{
	m_refcnt--;
	return *this;
}

ValueBase::~ValueBase()
{
}

const std::type_info& ValueBase::getType() const
{
	return m_type;
}

const std::string& ValueBase::getName() const
{
	return m_name;
}

const int &ValueBase::getRefCnt() const
{
	return m_refcnt;
}

const bool ValueBase::checkType(const std::type_info& typeRequested) const
{
	const bool ret = (typeRequested == m_type);
	if (!ret) fprintf(stderr, "Incorrect type requested for %s, requested %s, should be %s\n", m_name.length() ? m_name.c_str() : "[anonymous]", typeRequested.name(), typeid(m_type).name());
	return ret;
}


ValueUndefined::ValueUndefined(std::string n)
 : ValueBase(typeid(void), n)
{
}

ValueUndefined::ValueUndefined(const ValueUndefined &v)
 : ValueBase(typeid(void), v.getName())
{
}

ValueUndefined::~ValueUndefined()
{
}

const std::string ValueUndefined::get() const
{
	return "undefined";
}

const std::string ValueUndefined::toJson() const
{
	return get();
}

TO_JSON_TPL_PRIMITIVE(int)
TO_JSON_TPL_PRIMITIVE(long)
TO_JSON_TPL_PRIMITIVE(short)
TO_JSON_TPL(char, "'" << m_value << "'")
TO_JSON_TPL_PRIMITIVE(unsigned int)
TO_JSON_TPL_PRIMITIVE(unsigned long)
TO_JSON_TPL_PRIMITIVE(unsigned short)
TO_JSON_TPL(unsigned char, (unsigned int)m_value)
TO_JSON_TPL_PRIMITIVE(double)
TO_JSON_TPL(std::string, "'" << m_value << "'")
TO_JSON_TPL(bool, ((m_value) ? "true" : "false"))
