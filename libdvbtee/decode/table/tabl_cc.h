/*****************************************************************************
 * Copyright (C) 2011-2015 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; atsc_etther
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  cc110-1301  USA
 *
 *****************************************************************************/

#ifndef _ATSC_ETT_H__
#define _ATSC_ETT_H__

#include "table.h"

#ifndef _ATSC_ETT_H
#include "dvbpsi/atsc_ett.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* ETT (Table) */

class atsc_ett: public Table/*<dvbpsi_atsc_ett_t>*/ {
public:
	atsc_ett(Decoder *, TableWatcher*);
	atsc_ett(Decoder *, TableWatcher*, const dvbpsi_atsc_ett_t * const);
	virtual ~atsc_ett();

	void store(const dvbpsi_atsc_ett_t * const);

	static bool ingest(TableStore *s, const dvbpsi_atsc_ett_t * const t, TableWatcher *w = NULL);

	const decoded_atsc_ett_t& getDecodedETT() const { return decoded_ett; }

private:
	decoded_atsc_ett_t   decoded_ett;
};

}

}

#endif /* _ATSC_ETT_H__ */
