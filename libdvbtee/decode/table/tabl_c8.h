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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  c8110-1301  USA
 *
 *****************************************************************************/

#ifndef _VCT_H__
#define _VCT_H__

#include "table.h"

#ifndef _ATSC_VCT_H
#include "dvbpsi/atsc_vct.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* VCT (Table) */

class vct: public Table/*<dvbpsi_atsc_vct_t>*/ {
TABLE_DECODER_TPL
public:
	vct(Decoder *, TableWatcher*);
	vct(Decoder *, TableWatcher*, const dvbpsi_atsc_vct_t * const);
	virtual ~vct();

	void store(const dvbpsi_atsc_vct_t * const);

	static bool ingest(TableStore *s, const dvbpsi_atsc_vct_t * const t, TableWatcher *w = NULL);

	const decoded_vct_t& getDecodedVCT() const { return decoded_vct; }

private:
	decoded_vct_t   decoded_vct;
};

class vctCh: public TableDataComponent {
public:
	vctCh(decoded_vct_t&, Decoder*, const dvbpsi_atsc_vct_channel_t * const);
	virtual ~vctCh();
};

}

}

#endif /* _VCT_H__ */
