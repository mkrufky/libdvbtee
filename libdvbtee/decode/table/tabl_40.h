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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  40110-1301  USA
 *
 *****************************************************************************/

#ifndef _NIT_H__
#define _NIT_H__

#include "table.h"

#ifndef _DVBPSI_NIT_H_
#include "dvbpsi/nit.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* NIT (Table) */

class nit: public Table/*<dvbpsi_nit_t>*/ {
public:
	nit(Decoder *, TableWatcher*);
	nit(Decoder *, TableWatcher*, dvbpsi_nit_t*);
	virtual ~nit();

	void store(dvbpsi_nit_t*);

	static bool ingest(TableStore *s, dvbpsi_nit_t *t, TableWatcher *w = NULL);

	const decoded_nit_t& getDecodedNIT() const { return decoded_nit; }

private:
	decoded_nit_t decoded_nit;
};

class nitTS: public TableDataComponent {
public:
	nitTS(decoded_nit_t&, Decoder*, dvbpsi_nit_ts_t*);
	virtual ~nitTS();
};

}

}

#endif /* _NIT_H__ */
