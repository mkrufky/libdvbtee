/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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
#include "descript.h"

namespace dvbtee {

namespace decode {


class TableWatcher;

class TableBase: public Decoder {
public:
	const uint8_t& getTableid() const;
	void reset();

protected:
	TableBase(Decoder*, std::string&, uint8_t, TableWatcher*);
	virtual ~TableBase();

	TableWatcher *m_watcher;

	DescriptorStore descriptors;

private:
	const uint8_t m_tableid;

	void init();
};

class Table: public TableBase {
public:
	Table(Decoder*, std::string&, uint8_t, TableWatcher*);
	virtual ~Table();
};

class TableWatcher {
public:
	TableWatcher() {}
	virtual ~TableWatcher() {}

	virtual void updateTable(uint8_t tId, Table*) = 0;
};

class TableDataComponent: public LinkedDecoder {
public:
	TableDataComponent(Decoder*, std::string&);
	virtual ~TableDataComponent();

	DescriptorStore descriptors;
};

#define PsiTable_CONSTRUCTORTEMPLATE 0
struct PsiTable {
#if PsiTable_CONSTRUCTORTEMPLATE
    PsiTable(const PsiTable& o) : m_priv(o.m_priv) { }

    template<typename T> PsiTable(const T* p) : m_priv(p) { }
#else
    PsiTable() : m_priv(NULL) { }

    template<typename T> void Set(const T* p)
    {
        m_priv = p;
    }
#endif
    template<typename T> const T* Get() const
    {
        return (!m_priv) ? NULL : ((T*)m_priv);
    }
private:
    const void *m_priv;
};

class TableStore {
public:
	TableStore(Decoder *);
	~TableStore();

	bool add(uint8_t, PsiTable&, TableWatcher* watcher = NULL);

#if PsiTable_CONSTRUCTORTEMPLATE
	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher = NULL)
	{
		return __add(tableid, p_table, watcher);
	}
#else
	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher = NULL)
	{
		PsiTable psiTable;
		psiTable.Set<T>(p_table);
		return add(tableid, psiTable, watcher);
	}
#endif

	template <typename T> bool add(T*, TableWatcher* w = NULL);

	template<typename T, class C>
	bool update(uint8_t tableid, T* p_table)
	{
		std::vector<Table*> V = get(tableid);
		ssize_t s = V.size();
		if (s > 1) __log_printf(stderr, "TABLE: %02x %zu collected, something is wrong\n", tableid, s);
		if (s) {
			__log_printf(stderr, "UPDATING TABLE %02x\n", tableid);
			C *t = (C*)V[s-1];
			t->reset();
			t->store(p_table);
			return true;
		}
		return false;
	}

	template<typename T, class C>
	bool setOnly(uint8_t tableid, T* p_table, TableWatcher* watcher = NULL)
	{
		return (update<T,C>(tableid, p_table)) ? true : add(tableid, p_table, watcher);
	}

	template <typename T> bool setOnly(T*, TableWatcher* w = NULL);


	template<typename T, class C>
	bool ingest(T *p_table, TableWatcher *watcher = NULL)
	{
		return C::ingest(this, p_table, watcher);
	}

	template <typename T> bool ingest(T*, TableWatcher* w = NULL);

	const std::vector<Table*> get(uint8_t) const;

private:
	Decoder *m_parent;
	std::multimap<uint8_t, Table*> m_store;

#if PsiTable_CONSTRUCTORTEMPLATE
	bool __add(uint8_t, PsiTable, TableWatcher*);
#endif
};

template <uint8_t TABLEID, class T, typename S>
class LinkedTable : public T {
public:
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

	~LinkedTable()
	{
		Decoder *parent = T::getParent();
		if (parent) parent->unlinkChild(m_linkedIdx);
	}

	virtual int getMapIndex() const
	{
		return m_linkedIdx;
	}

private:
	int m_linkedIdx;

	inline void linkParent(Decoder *parent)
	{
		if (parent) m_linkedIdx = parent->linkChild(this);
	}
};

class TableBaseFactory {
public:
	virtual Table *create(Decoder *parent, TableWatcher *watcher) const = 0;
	virtual Table *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher) const = 0;
protected:
	TableBaseFactory() {}
	~TableBaseFactory() {}
};

class TableRegistry {
public:
	static TableRegistry& instance();

	bool registerFactory(uint8_t, TableBaseFactory*);
	const TableBaseFactory* getFactory(uint8_t) const;

	int count() const;
private:
	TableRegistry();
	~TableRegistry();

	mutable pthread_mutex_t m_mutex;
	std::map <uint8_t, const TableBaseFactory*> m_factories;
};

template <uint8_t TABLEID, typename S, class T>
class TableFactory : public TableBaseFactory {
public:
	static const TableFactory<TABLEID, S, T>& instance()
	{
		static TableFactory<TABLEID, S, T> INSTANCE;
		return INSTANCE;
	}
	virtual T *create(Decoder *parent, TableWatcher *watcher) const
	{
		return new LinkedTable<TABLEID,T,S>(parent, watcher);
	}
	virtual T *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher) const
	{
		return new LinkedTable<TABLEID,T,S>(parent, inTable, watcher);
	}
private:
	TableFactory() {
		bool TableFactoryRegistration = TableRegistry::instance().registerFactory(TABLEID, this);
		assert(TableFactoryRegistration);
	}
	~TableFactory() {}
};

#define TABLE_DECODER_TPL \
	static void dummy();

#define REGISTER_TABLE_FACTORY(tableid, psitable, decoder) \
	namespace dvbtee {\
	namespace decode {\
	\
	template <>\
	bool TableStore::add<psitable>(psitable *p, TableWatcher *w) { return add(tableid, p, w); }\
	\
	template <>\
	bool TableStore::setOnly<psitable>(psitable *p, TableWatcher *w) { return setOnly<psitable, decoder>(tableid, p, w); }\
	\
	template <>\
	bool TableStore::ingest<psitable>(psitable *p, TableWatcher *w) { return ingest<psitable, decoder>(p, w); }\
	\
	}}\
	\
	static volatile const TableFactory<tableid, psitable, decoder> &__TablFactory = TableFactory<tableid, psitable, decoder>::instance();\
	\
	void decoder::dummy() { __TablFactory.instance(); }
}

}

#endif /* __TABLE_H__ */
