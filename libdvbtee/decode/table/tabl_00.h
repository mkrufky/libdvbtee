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

#ifndef _PAT_H__
#define _PAT_H__

#include "table.h"

#ifndef _DVBPSI_PAT_H_
#include "dvbpsi/pat.h"
#endif

namespace dvbtee {

namespace decode {

/* PAT (Table) */

typedef std::map<uint16_t, uint16_t> map_decoded_pat_programs; /* program number, pid */

class pat: public Table/*<dvbpsi_pat_t>*/ {
public:
	pat(Decoder *);
	pat(Decoder *, TableWatcher*);
	pat(Decoder *, TableWatcher*, dvbpsi_pat_t*);
	pat(Decoder *, dvbpsi_pat_t*);
	virtual ~pat();

	const uint16_t& getTsId() const { return m_ts_id; }
	const uint8_t& getVersion() const { return m_version; }
	const map_decoded_pat_programs& getPrograms() const { return m_programs; }

	void store(dvbpsi_pat_t*);

	static bool ingest(TableStore *s, dvbpsi_pat_t *t, TableWatcher *w = NULL);
private:
	uint16_t			m_ts_id;
	uint8_t				m_version;
	map_decoded_pat_programs	m_programs;
};

}

}

#endif /* _PAT_H__ */
