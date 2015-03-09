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
	TableComponent(Decoder*, std::string&);
	virtual ~TableComponent();

	dvbtee::decode::DescriptorStore descriptors;
};


class TableBase: public TableComponent {
public:
	TableBase(Decoder*, std::string&, uint8_t);
	TableBase(Decoder*, std::string&, uint8_t, TableWatcher*);
	virtual ~TableBase();

	uint8_t getTableid();
protected:
	TableWatcher *m_watcher;
private:
	const uint8_t m_tableid;
};

class Table: public TableBase {
public:
	Table(Decoder*, std::string&, uint8_t);
	Table(Decoder*, std::string&, uint8_t, TableWatcher*);
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

#define PsiTable_CONSTRUCTORTEMPLATE 1
struct PsiTable {
#if PsiTable_CONSTRUCTORTEMPLATE
    template<typename T> PsiTable(TableTypeCarrier<T> inT) { m_priv = &inT; }
#else
    template<typename T> void Set(TableTypeCarrier<T> inT) { m_priv = &inT; }
#endif
    template<typename T> T* Get() { return ((TableTypeCarrier<T>*)m_priv)->Get(); }

private:
    TableTypeCarrierBase* m_priv;
};

class TableStore {
public:
	TableStore(Decoder *);
	~TableStore();

#if PsiTable_CONSTRUCTORTEMPLATE
	bool add(uint8_t, PsiTable);
	bool add(uint8_t, PsiTable, TableWatcher*);

	template<typename T>
	bool add(uint8_t tableid, T* p_table) { return add(tableid, PsiTable(TableTypeCarrier<T>(p_table))); }
	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher) { return add(tableid, PsiTable(TableTypeCarrier<T>(p_table)), watcher); }
#else
	bool add(uint8_t, PsiTable&);
	bool add(uint8_t, PsiTable&, TableWatcher*);

	template<typename T>
	bool add(uint8_t tableid, T* p_table) { PsiTable psiTable; psiTable.Set<T>(p_table); return add(tableid, psiTable); }
	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher) { PsiTable psiTable; psiTable.Set<T>(p_table); return add(tableid, psiTable, watcher); }
#endif

	template <typename T> bool add(T*);
	template <typename T> bool add(T*, TableWatcher*);

	template<typename T, class C>
	bool update(uint8_t tableid, T* p_table)
	{
		std::vector<Table*> V = get(tableid);
		ssize_t s = V.size();
		if (s > 1) printf("TABLE: %02x %ld collected, something is wrong", tableid, s);
		if (s) {
			printf("UPDATING TABLE %02x", tableid);
			C *t = (C*)V[s-1];
			t->store(p_table);
			return true;
		}
		return false;
	}

#if PsiTable_CONSTRUCTORTEMPLATE
	template<typename T, class C>
	bool setOnly(uint8_t tableid, T* p_table)
	{
		return (update<T,C>(tableid, p_table)) ? true : add(p_table);
	}
	template<typename T, class C>
	bool setOnly(uint8_t tableid, T* p_table, TableWatcher* watcher)
	{
		return (update<T,C>(tableid, p_table)) ? true : add(p_table, watcher);
	}
#else
	template<typename T, class C>
	bool setOnly(uint8_t tableid, T* p_table)
	{
		if (update<T,C>(tableid, p_table)) return true;

		PsiTable psiTable;
		psiTable.Set<T>(p_table);
		return add(tableid, psiTable);
	}
	template<typename T, class C>
	bool setOnly(uint8_t tableid, T* p_table, TableWatcher* watcher)
	{
		if (update<T,C>(tableid, p_table)) return true;

		PsiTable psiTable;
		psiTable.Set<T>(p_table);
		return add(tableid, psiTable, watcher);
	}
#endif

	template <typename T> bool setOnly(T*);
	template <typename T> bool setOnly(T*, TableWatcher*);

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

	LinkedTable(Decoder *parent, PsiTable& inTable, TableWatcher *watcher)
	 : T(parent, watcher, inTable.Get<S>())
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	LinkedTable(Decoder *parent, PsiTable& inTable)
	 : T(parent, inTable.Get<S>())
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
	virtual Table *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher) = 0;
	virtual Table *create(Decoder *parent, PsiTable& inTable) = 0;
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
	virtual T *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher)
	{
		return new LinkedTable<TABLEID,T,S>(parent, inTable, watcher);
	}
	virtual T *create(Decoder *parent, PsiTable& inTable)
	{
		return new LinkedTable<TABLEID,T,S>(parent, inTable);
	}
private:
	TableFactory() {
		bool TableFactoryRegistration = TableRegistry::instance().registerFactory(TABLEID, this);
		assert(TableFactoryRegistration);
	}
	~TableFactory() {}
};

#define REGISTER_TABLE_FACTORY(tableid, psitable, decoder) \
	namespace dvbtee {\
	namespace decode {\
	\
	template <>\
	bool TableStore::add<psitable>(psitable *p) { return add(tableid, p); }\
	template <>\
	bool TableStore::add<psitable>(psitable *p, TableWatcher *w) { return add(tableid, p, w); }\
	\
	template <>\
	bool TableStore::setOnly<psitable>(psitable *p) { return setOnly<psitable, decoder>(tableid, p); }\
	template <>\
	bool TableStore::setOnly<psitable>(psitable *p, TableWatcher *w) { return setOnly<psitable, decoder>(tableid, p, w); }\
	\
	}}\
	\
	static TableFactory<tableid, psitable, decoder> &__TablFactory = TableFactory<tableid, psitable, decoder>::instance()
}

}

#endif /* __TABLE_H__ */
