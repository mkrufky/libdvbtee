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

#include "table.h"

using namespace dvbtee::decode;

TableDataComponent::TableDataComponent(Decoder *parent, std::string &name)
 : LinkedDecoder(parent, name)
 , descriptors(this)
{
}

TableDataComponent::~TableDataComponent()
{
}


void TableBase::init()
{
	set("tableId", m_tableid);
	set("tableName", getDecoderName());
}

TableBase::TableBase(Decoder *parent, std::string &name, uint8_t tableid, TableWatcher *watcher)
 : Decoder(parent, name)
 , m_watcher(watcher)
 , descriptors(this)
 , m_tableid(tableid)
{
	init();
}

TableBase::~TableBase()
{
}

const uint8_t& TableBase::getTableid() const
{
	return m_tableid;
}

void TableBase::reset()
{
	clear();
	init();
}


Table::Table(Decoder *parent, std::string &name, uint8_t tableid, TableWatcher *watcher)
 : TableBase(parent, name, tableid, watcher)
{
}

Table::~Table()
{
}


bool TableRegistry::registerFactory(uint8_t tableid, TableBaseFactory *factory)
{
	pthread_mutex_lock(&m_mutex);

	if (m_factories.count(tableid)) {
		pthread_mutex_unlock(&m_mutex);
		return false;
	}

	m_factories.insert( std::pair<uint8_t, const TableBaseFactory*>(tableid, factory) );
#if DBG_DECODER_INSERTION
	fprintf(stderr, "inserted 0x%02x, %p, %ld table decoders present\n", tableid, factory, m_factories.size());
#endif
	pthread_mutex_unlock(&m_mutex);
	return true;
}

const TableBaseFactory* TableRegistry::getFactory(uint8_t tableid) const
{
	pthread_mutex_lock(&m_mutex);
	std::map <uint8_t, const TableBaseFactory*>::const_iterator it = m_factories.find(tableid);
	pthread_mutex_unlock(&m_mutex);

	return (it == m_factories.end()) ? NULL : it->second;
}

int TableRegistry::count() const
{
	fprintf(stderr, "%ld table decoders present:", m_factories.size());
	for (std::map <uint8_t, const TableBaseFactory*>::const_iterator it = m_factories.begin(); it != m_factories.end(); ++it) {
		fprintf(stderr, " 0x%02x", it->first);
	}
	fprintf(stderr, "\n");
	return m_factories.size();
}

TableRegistry::TableRegistry()
{
	pthread_mutex_init(&m_mutex, 0);
}

TableRegistry::~TableRegistry()
{
	pthread_mutex_destroy(&m_mutex);
}

TableRegistry &TableRegistry::instance()
{
	static TableRegistry INSTANCE;
	return INSTANCE;
}

TableStore::TableStore(Decoder *parent)
 : m_parent(parent)
{
}

TableStore::~TableStore()
{
}

#if PsiTable_CONSTRUCTORTEMPLATE
bool TableStore::__add(uint8_t tableid, PsiTable table, TableWatcher *watcher)
{
	return add(tableid, table, watcher);
}
#endif

bool TableStore::add(uint8_t id, PsiTable &inTable, TableWatcher *watcher)
{
	Table *t = NULL;

	const TableBaseFactory* f = TableRegistry::instance().getFactory(id);
	if (f) t = f->create(m_parent, inTable, watcher);

	if (t) m_store.insert( std::pair<uint8_t, Table*>(t->getTableid(), t) );
	return (t != NULL);
}


const std::vector<Table*> TableStore::get(uint8_t tableid) const
{
	std::vector<Table*> ret;
	std::pair <std::multimap<uint8_t, Table*>::const_iterator, std::multimap<uint8_t, Table*>::const_iterator> range;

	range = m_store.equal_range(tableid);

	for (std::multimap<uint8_t, Table*>::const_iterator it=range.first; it!=range.second; ++it)
		ret.push_back(it->second);

	return ret;
}
