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

#ifndef _TOT_H__
#define _TOT_H__

#include "table.h"

#ifndef _DVBPSI_TOT_H_
#include "dvbpsi/tot.h"
#endif

namespace dvbtee {

namespace decode {

/* TDT (Time and Date Table)/TOT (Time Offset Table) */

class tot: public Table/*<dvbpsi_tot_t>*/ {
public:
	tot(Decoder *, TableWatcher*);
	tot(Decoder *, TableWatcher*, dvbpsi_tot_t*);
	tot(Decoder *, dvbpsi_tot_t*);
	virtual ~tot();

	void store(dvbpsi_tot_t*);

	static bool ingest(TableStore *s, dvbpsi_tot_t *t, TableWatcher *w = NULL);
};

}

}

#endif /* _TOT_H__ */
