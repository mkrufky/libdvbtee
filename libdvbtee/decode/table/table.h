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

#include "decode/decoder.h"
#include "decode/descriptor/descriptor.h"

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

class TableTypeCarrierBase {};

template<class T>
class TableTypeCarrier: public TableTypeCarrierBase {
public:
	explicit TableTypeCarrier(const T * const p) : m_p(p) {}

	const T * const Get() const { return m_p; }
private:
	const T * const m_p;
};

#define PsiTable_CONSTRUCTORTEMPLATE 0
struct PsiTable {
    explicit PsiTable() : m_priv(NULL) { }
#if PsiTable_CONSTRUCTORTEMPLATE
    template<typename T> explicit PsiTable(const TableTypeCarrier<T> inT) { m_priv = &inT; }
#else
    template<typename T> void Set(const TableTypeCarrier<T> inT) { m_priv = &inT; }
#endif
    template<typename T> const T * const Get() const { return (!m_priv) ? NULL : ((TableTypeCarrier<T>*)m_priv)->Get(); }

private:
    const TableTypeCarrierBase *m_priv;
};

class TableStore {
public:
	TableStore(Decoder *);
	~TableStore();

#if PsiTable_CONSTRUCTORTEMPLATE
	bool add(uint8_t, PsiTable, TableWatcher* watcher = NULL);

	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher = NULL)
	{
		return add(tableid, PsiTable(TableTypeCarrier<T>(p_table)), watcher);
	}
#else
	bool add(uint8_t, PsiTable&, TableWatcher* watcher = NULL);

	template<typename T>
	bool add(uint8_t tableid, T* p_table, TableWatcher* watcher = NULL)
	{
		PsiTable psiTable;
		TableTypeCarrier<T> inT(p_table);
		psiTable.Set<T>(inT);
		return add(tableid, psiTable, watcher);
	}
#endif

	template <typename T> bool add(T*, TableWatcher* w = NULL);

	template<typename T, class C>
	bool update(uint8_t tableid, T* p_table)
	{
		std::vector<Table*> V = get(tableid);
		ssize_t s = V.size();
		if (s > 1) printf("TABLE: %02x %zu collected, something is wrong\n", tableid, s);
		if (s) {
			printf("UPDATING TABLE %02x\n", tableid);
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

	virtual int getMapIndex() const { return m_linkedIdx; }

private:
	int m_linkedIdx;

	inline void linkParent(Decoder *parent) { if (parent) m_linkedIdx = parent->linkChild(this); }
};

class TableBaseFactory {
public:
	virtual Table *create(Decoder *parent, TableWatcher *watcher) = 0;
	virtual Table *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher) = 0;
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
	virtual T *create(Decoder *parent, TableWatcher *watcher)
	{
		return new LinkedTable<TABLEID,T,S>(parent, watcher);
	}
	virtual T *create(Decoder *parent, PsiTable& inTable, TableWatcher *watcher)
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
	static TableFactory<tableid, psitable, decoder> &__TablFactory = TableFactory<tableid, psitable, decoder>::instance()
}

}

#endif /* __TABLE_H__ */
