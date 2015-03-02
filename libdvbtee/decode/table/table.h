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


class TableWatcher;

class TableComponent: public Decoder {
public:
	TableComponent(Decoder*);
	virtual ~TableComponent();

	dvbtee::decode::DescriptorStore descriptors;
};


class TableBase: public TableComponent {
public:
	TableBase(Decoder*, uint8_t);
	TableBase(Decoder*, uint8_t, TableWatcher*);
	virtual ~TableBase();

	uint8_t getTableid();
protected:
	TableWatcher *m_watcher;
private:
	const uint8_t m_tableid;
};

class Table: public TableBase {
public:
	Table(Decoder*, uint8_t);
	Table(Decoder*, uint8_t, TableWatcher*);
	virtual ~Table();
};

class TableWatcher {
public:
	TableWatcher() {}
	virtual ~TableWatcher() {}

	virtual void updateTable(uint8_t tId, Table*) = 0;
};

class TableTypeCarrierBase {};

template<class T>
class TableTypeCarrier: public TableTypeCarrierBase {
public:
	TableTypeCarrier(T* p) : m_p(p) {}

	T* Get() { return m_p; }
private:
	T* m_p;
};

struct PsiTable {
    //template<typename T> PsiTable(TableTypeCarrier<T> inT) { m_priv = &inT; }
    template<typename T> void Set(TableTypeCarrier<T> inT) { m_priv = &inT; }

    //template<typename T> T* Get() { TableTypeCarrier<T> *t = (TableTypeCarrier<T>*)m_priv; return t->Get(); }
    template<typename T> T* Get() { return ((TableTypeCarrier<T>*)m_priv)->Get(); }

private:
    TableTypeCarrierBase* m_priv;
};

class TableStore {
public:
	TableStore(Decoder *);
	~TableStore();

	bool add(uint8_t, PsiTable*);
	bool add(uint8_t, PsiTable*, TableWatcher*);

	std::vector<Table*> get(uint8_t);

private:
	Decoder *m_parent;
	std::multimap<uint8_t, Table*> m_store;
};

template <uint8_t TABLEID, class T, typename S>
class LinkedTable : public T {
public:
	LinkedTable(Decoder *parent)
	 : T(parent)
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	LinkedTable(Decoder *parent, TableWatcher *watcher)
	 : T(parent, watcher)
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	LinkedTable(Decoder *parent, TableWatcher *watcher, PsiTable *p_Table)
	 : T(parent, watcher, p_Table->Get<S>())
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	LinkedTable(Decoder *parent, PsiTable *p_Table)
	 : T(parent, p_Table->Get<S>())
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	~LinkedTable()
	{
		Decoder *parent = T::getParent();
		if (parent) parent->unlinkChild(m_linkedIdx);
	}

	virtual int getMapIndex() { return m_linkedIdx; }

private:
	int m_linkedIdx;

	inline void linkParent(Decoder *parent) { if (parent) m_linkedIdx = parent->linkChild(this); }
};

class TableBaseFactory {
public:
	virtual Table *create(Decoder *parent) = 0;
	virtual Table *create(Decoder *parent, TableWatcher *watcher) = 0;
	virtual Table *create(Decoder *parent, TableWatcher *watcher, PsiTable *p_Table) = 0;
	virtual Table *create(Decoder *parent, PsiTable *p_Table) = 0;
protected:
	TableBaseFactory() {}
	~TableBaseFactory() {}
};

class TableRegistry {
public:
	static TableRegistry& instance();

	bool registerFactory(uint8_t, TableBaseFactory*);
	TableBaseFactory *getFactory(uint8_t);

private:
	TableRegistry();
	~TableRegistry();

	pthread_mutex_t m_mutex;
	std::map <uint8_t, TableBaseFactory*> m_factories;
};

template <uint8_t TABLEID, typename S, class T>
class TableFactory : public TableBaseFactory {
public:
	static TableFactory<TABLEID, S, T>& instance()
	{
		static TableFactory<TABLEID, S, T> INSTANCE;
		return INSTANCE;
	}
	virtual T *create(Decoder *parent)
	{
		return new LinkedTable<TABLEID,T,S>(parent);
	}
	virtual T *create(Decoder *parent, TableWatcher *watcher)
	{
		return new LinkedTable<TABLEID,T,S>(parent, watcher);
	}
	virtual T *create(Decoder *parent, TableWatcher *watcher, PsiTable *p_Table)
	{
		return new LinkedTable<TABLEID,T,S>(parent, watcher, p_Table);
	}
	virtual T *create(Decoder *parent, PsiTable *p_Table)
	{
		return new LinkedTable<TABLEID,T,S>(parent, p_Table);
	}
private:
	TableFactory() {
		bool TableFactoryRegistration = TableRegistry::instance().registerFactory(TABLEID, this);
		assert(TableFactoryRegistration);
	}
	~TableFactory() {}
};

#define REGISTER_TABLE_FACTORY(tableid, psitable, decoder) static TableFactory<tableid, psitable, decoder> &__TablFactory = TableFactory<tableid, psitable, decoder>::instance()



}

}

#endif /* __TABLE_H__ */
