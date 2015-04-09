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

#include "functions.h"
#include "table.h"

#include "tabl_00.h"
#include "tabl_02.h"
#include "tabl_40.h"
#include "tabl_42.h"
#include "tabl_4e.h"
#include "tabl_70.h"
#include "tabl_c7.h"
#include "tabl_c8.h"
#include "tabl_cb.h"
#include "tabl_cc.h"
#include "tabl_cd.h"

namespace dvbtee {
namespace decode {

#if PsiTable_CONSTRUCTORTEMPLATE
template<typename T> PsiTable::PsiTable(TableTypeCarrier<T> inT) { m_priv = &inT; }
#else
template<typename T> void PsiTable::Set(TableTypeCarrier<T> inT) { m_priv = &inT; }
#endif
template<typename T> T* PsiTable::Get() { return ((TableTypeCarrier<T>*)m_priv)->Get(); }


#if PsiTable_CONSTRUCTORTEMPLATE
template<typename T>
bool TableStore::add(uint8_t tableid, T* p_table, TableWatcher* watcher) { return add(tableid, PsiTable(TableTypeCarrier<T>(p_table)), watcher); }
#else
template<typename T>
bool TableStore::add(uint8_t tableid, T* p_table, TableWatcher* watcher) { PsiTable psiTable; psiTable.Set<T>(p_table); return add(tableid, psiTable, watcher); }
#endif

template<typename T, class C>
bool TableStore::update(uint8_t tableid, T* p_table)
{
	std::vector<Table*> V = get(tableid);
	ssize_t s = V.size();
	if (s > 1) printf("TABLE: %02x %ld collected, something is wrong\n", tableid, s);
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
#if PsiTable_CONSTRUCTORTEMPLATE
bool TableStore::setOnly(uint8_t tableid, T* p_table, TableWatcher* watcher)
{
	return (update<T,C>(tableid, p_table)) ? true : add(p_table, watcher);
}
#else
bool TableStore::setOnly(uint8_t tableid, T* p_table, TableWatcher* watcher)
{
	if (update<T,C>(tableid, p_table)) return true;

	PsiTable psiTable;
	psiTable.Set<T>(p_table);
	return add(tableid, psiTable, watcher);
}
#endif

template<typename T, class C>
bool TableStore::ingest(T *p_table, TableWatcher *watcher)
{
	return C::ingest(this, p_table, watcher);
}



}}

#define IMPL_TABLESTORE_TMPL(C,T) \
template bool TableStore::add<T>(uint8_t, T*, TableWatcher*); \
template bool TableStore::setOnly<T,C>(uint8_t, T*, TableWatcher*); \
template bool TableStore::ingest<T,C>(T*, TableWatcher*); \
template PsiTable::PsiTable<T>(TableTypeCarrier<T> inT); \
/*template void PsiTable::Set<T>(TableTypeCarrier<T> inT);*/ \
template T* PsiTable::Get<T>()


using namespace dvbtee::decode;
IMPL_TABLESTORE_TMPL(pat, dvbpsi_pat_t);
IMPL_TABLESTORE_TMPL(pmt, dvbpsi_pmt_t);
IMPL_TABLESTORE_TMPL(nit, dvbpsi_nit_t);
IMPL_TABLESTORE_TMPL(sdt, dvbpsi_sdt_t);
IMPL_TABLESTORE_TMPL(tot, dvbpsi_tot_t);
IMPL_TABLESTORE_TMPL(eit, dvbpsi_eit_t);
IMPL_TABLESTORE_TMPL(mgt, dvbpsi_atsc_mgt_t);
IMPL_TABLESTORE_TMPL(vct, dvbpsi_atsc_vct_t);
IMPL_TABLESTORE_TMPL(atsc_eit, dvbpsi_atsc_eit_t);
IMPL_TABLESTORE_TMPL(atsc_ett, dvbpsi_atsc_ett_t);
IMPL_TABLESTORE_TMPL(stt, dvbpsi_atsc_stt_t);
