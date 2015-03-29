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

TableComponent::TableComponent(Decoder *parent, std::string &name)
 : Decoder(parent, name)
 , descriptors(this)
{
	//
}

TableComponent::~TableComponent()
{
	//
}


void TableBase::init()
{
	set("tableId", m_tableid);
}

TableBase::TableBase(Decoder *parent, std::string &name, uint8_t tableid)
 : TableComponent(parent, name)
 , m_watcher(NULL)
 , m_tableid(tableid)
{
	init();
}

TableBase::TableBase(Decoder *parent, std::string &name, uint8_t tableid, TableWatcher *watcher)
 : TableComponent(parent, name)
 , m_watcher(watcher)
 , m_tableid(tableid)
{
	init();
}

TableBase::~TableBase()
{
	//
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


Table::Table(Decoder *parent, std::string &name, uint8_t tableid)
 : TableBase(parent, name, tableid)
{
	//
}

Table::Table(Decoder *parent, std::string &name, uint8_t tableid, TableWatcher *watcher)
 : TableBase(parent, name, tableid, watcher)
{
	//
}

Table::~Table()
{
	//
}


bool TableRegistry::registerFactory(uint8_t tableid, TableBaseFactory *factory)
{
	pthread_mutex_lock(&m_mutex);

	if (m_factories.count(tableid)) {
		pthread_mutex_unlock(&m_mutex);
		return false;
	}

	m_factories.insert( std::pair<uint8_t, TableBaseFactory*>(tableid,factory) );

	fprintf(stderr, "inserted 0x%02x, %p, %ld table decoders present\n", tableid, factory, m_factories.size());

	pthread_mutex_unlock(&m_mutex);
	return true;
}

TableBaseFactory *TableRegistry::getFactory(uint8_t tableid)
{
	pthread_mutex_lock(&m_mutex);

	if (!m_factories.count(tableid)) {
		pthread_mutex_unlock(&m_mutex);
		return NULL;
	}

	TableBaseFactory* ret = m_factories[tableid];

	pthread_mutex_unlock(&m_mutex);

	return ret;
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
	//
}

TableStore::~TableStore()
{
	//
}

#if PsiTable_CONSTRUCTORTEMPLATE
bool TableStore::add(uint8_t id, PsiTable p_table)
#else
bool TableStore::add(uint8_t id, PsiTable &p_table)
#endif
{
	Table *t = NULL;

	TableBaseFactory* f = TableRegistry::instance().getFactory(id);
	if (f) t = f->create(m_parent, p_table);

	if (t) m_store.insert( std::pair<uint8_t, Table*>(t->getTableid(), t) );
	return (t != NULL);
}

#if PsiTable_CONSTRUCTORTEMPLATE
bool TableStore::add(uint8_t id, PsiTable inTable, TableWatcher *watcher)
#else
bool TableStore::add(uint8_t id, PsiTable &inTable, TableWatcher *watcher)
#endif
{
	Table *t = NULL;

	TableBaseFactory* f = TableRegistry::instance().getFactory(id);
	if (f) t = f->create(m_parent, inTable, watcher);

	if (t) m_store.insert( std::pair<uint8_t, Table*>(t->getTableid(), t) );
	return (t != NULL);
}


std::vector<Table *> TableStore::get(uint8_t tableid)
{
	std::vector<Table *> ret;
	std::pair <std::multimap<uint8_t, Table*>::iterator, std::multimap<uint8_t, Table*>::iterator> range;

	range = m_store.equal_range(tableid);

	for (std::multimap<uint8_t, Table*>::iterator it=range.first; it!=range.second; ++it)
		ret.push_back(it->second);

	return ret;
}
