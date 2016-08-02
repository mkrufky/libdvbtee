/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; atsc_either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  cb110-1301  USA
 *
 *****************************************************************************/

#ifndef _ATSC_EIT_H__
#define _ATSC_EIT_H__

#include "table.h"

#ifndef _ATSC_EIT_H
#include "dvbpsi/atsc_eit.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* EIT (Table) */

class atsc_eit: public Table/*<dvbpsi_atsc_eit_t>*/ {
TABLE_DECODER_TPL
public:
	atsc_eit(Decoder *, TableWatcher*);
	atsc_eit(Decoder *, TableWatcher*, const dvbpsi_atsc_eit_t * const);
	virtual ~atsc_eit();

	void store(const dvbpsi_atsc_eit_t * const);

	static bool ingest(TableStore *s, const dvbpsi_atsc_eit_t * const t, TableWatcher *w = NULL);

	const decoded_atsc_eit_t& getDecodedEIT() const { return decoded_eit; }

private:
	decoded_atsc_eit_t   decoded_eit;
};

class atsc_eitEV: public TableDataComponent {
public:
	atsc_eitEV(decoded_atsc_eit_t&, Decoder*, const dvbpsi_atsc_eit_event_t * const);
	virtual ~atsc_eitEV();
};

}

}

#endif /* _ATSC_EIT_H__ */
