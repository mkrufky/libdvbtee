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

#include "handle.h"

using namespace valueobj;

static ReferencedValueUndefined& valueUndefined = ReferencedValueUndefined::instance();

Handle::Handle()
 : m_value(NULL) {}

Handle::~Handle()
{
  clear();
}

Handle::Handle(const Handle &hdl)
 : m_value(hdl.m_value)
{
  incRefCnt();
}

Handle::Handle(const ValueBase *v)
 : m_value((ValueBase*)v)
{
  incRefCnt();
}

Handle::Handle(ValueBase *v)
  : m_value(v)
{
  incRefCnt();
}

const ValueBase *Handle::set(const char *s)
{
  std::string key("");
  std::string val(s);
  return set((ValueBase*)new Value<std::string>(key, val));
}

const ValueBase *Handle::set(char *s)
{
  std::string key("");
  std::string val(s);
  return set((ValueBase*)new Value<std::string>(key, val));
}

const ValueBase *Handle::set(Handle &hdl)
{
  return set(hdl.m_value);
}

const ValueBase *Handle::set(ValueBase *val)
{
  clear();
  m_value = val;
  incRefCnt();
  return m_value;
}

const ValueBase *Handle::set(const ValueBase *val)
{
  return set((ValueBase*)val);
}

const ValueBase *Handle::get() const
{
  return m_value;
}

void Handle::clear()
{
  decRefCnt();
  m_value = NULL;
}

size_t Handle::size() const
{
  return (m_value == NULL) ? 0 : 1;
}

const std::string Handle::toJson() const
{
  if (m_value) return m_value->toJson();
  return "";
}

void Handle::incRefCnt() {
  if (m_value) ++(*m_value);
}

void Handle::decRefCnt()
{
  if (m_value) {
      if (!((--(*m_value)).getRefCnt())) {
          delete m_value;
        }
    }
}

Handle &Handle::operator=(ValueBase *v)
{
  set(v);
  return *this;
}

Handle &Handle::operator=(Handle &hdl)
{
  set(hdl);
  return *this;
}
