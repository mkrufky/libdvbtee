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

#ifndef __TABLE_H__
#define __TABLE_H__

#include <string>

#include "decoder.h"
#include "descriptor.h"

namespace dvbtee {

namespace decode {


class TableWatcher {
public:
	TableWatcher() {}
	virtual ~TableWatcher() {}

	virtual void updateTable(uint8_t tId) = 0;
};


class TableComponent: public Decoder {
public:
	TableComponent(Decoder*);
	virtual ~TableComponent();

	dvbtee::decode::DescriptorStore descriptors;
};


class TableBase: public TableComponent {
public:
	TableBase(Decoder*);
	TableBase(Decoder*, TableWatcher*);
	virtual ~TableBase();
protected:
	TableWatcher *m_watcher;
};

class Table: public TableBase {
public:
	Table(Decoder*);
	Table(Decoder*, TableWatcher*);
	virtual ~Table();
};


}

}

#endif /* __TABLE_H__ */
