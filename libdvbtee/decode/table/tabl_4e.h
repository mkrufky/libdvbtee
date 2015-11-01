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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  4e110-1301  USA
 *
 *****************************************************************************/

#ifndef _EIT_H__
#define _EIT_H__

#include "table.h"

#ifndef _DVBPSI_EIT_H_
#include "dvbpsi/eit.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* EIT (Table) */

class eit: public Table/*<dvbpsi_eit_t>*/ {
TABLE_DECODER_TPL
public:
	eit(Decoder *, TableWatcher*);
	eit(Decoder *, TableWatcher*, const dvbpsi_eit_t * const);
	virtual ~eit();

	void store(const dvbpsi_eit_t * const);

	static bool ingest(TableStore *s, const dvbpsi_eit_t * const t, TableWatcher *w = NULL);

	const decoded_eit_t& getDecodedEIT() const { return decoded_eit; }

private:
	decoded_eit_t decoded_eit;
};

class eitEV: public TableDataComponent {
public:
	eitEV(decoded_eit_t&, Decoder*, const dvbpsi_eit_event_t * const);
	virtual ~eitEV();
};

}

}

#endif /* _EIT_H__ */
