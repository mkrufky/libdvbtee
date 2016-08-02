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

#ifndef _STT_H__
#define _STT_H__

#include "table.h"

#include "dvbpsi/atsc_stt.h"

namespace dvbtee {

namespace decode {

/* System Time Table (ATSC) */

class stt: public Table/*<dvbpsi_atsc_stt_t>*/ {
TABLE_DECODER_TPL
public:
	stt(Decoder *, TableWatcher*);
	stt(Decoder *, TableWatcher*, const dvbpsi_atsc_stt_t * const);
	virtual ~stt();

	void store(const dvbpsi_atsc_stt_t * const);

	static bool ingest(TableStore *s, const dvbpsi_atsc_stt_t * const t, TableWatcher *w = NULL);
};

}

}

#endif /* _STT_H__ */
