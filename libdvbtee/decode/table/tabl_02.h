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

#ifndef _PMT_H__
#define _PMT_H__

#include "table.h"

#ifndef _DVBPSI_PMT_H_
#include "dvbpsi/pmt.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* PMT (Table) */

class pmt: public Table/*<dvbpsi_pmt_t>*/ {
public:
	pmt(Decoder *, TableWatcher*);
	pmt(Decoder *, TableWatcher*, dvbpsi_pmt_t*);
	virtual ~pmt();

	void store(dvbpsi_pmt_t*);

	static bool ingest(TableStore *s, dvbpsi_pmt_t *t, TableWatcher *w = NULL);

	const decoded_pmt_t& getDecodedPMT() const { return decoded_pmt; }

private:
	decoded_pmt_t decoded_pmt;
};

class pmtES: public TableDataComponent {
public:
	pmtES(decoded_pmt_t&, Decoder*, dvbpsi_pmt_es_t*);
	virtual ~pmtES();
};

}

}

#endif /* _PMT_H__ */
