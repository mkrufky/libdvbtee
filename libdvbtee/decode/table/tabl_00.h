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

#include "decode.h"

namespace dvbtee {

namespace decode {

/* PAT (Table) */

class pat: public Table/*<dvbpsi_pat_t>*/ {
TABLE_DECODER_TPL
public:
	pat(Decoder *, TableWatcher*);
	pat(Decoder *, TableWatcher*, const dvbpsi_pat_t * const);
	virtual ~pat();

	void store(const dvbpsi_pat_t * const);

	static bool ingest(TableStore *s, const dvbpsi_pat_t * const t, TableWatcher *w = NULL);
#if 0
	const uint16_t& getTsId() const { return decoded_pat.ts_id; }
	const uint8_t& getVersion() const { return decoded_pat.version; }
	const map_decoded_pat_programs& getPrograms() const { return decoded_pat.programs; }
#endif
	const decoded_pat_t& getDecodedPAT() const { return decoded_pat; }

private:
	decoded_pat_t decoded_pat;
};

class patProgram: public TableDataComponent {
public:
	patProgram(Decoder*, const dvbpsi_pat_program_t * const);
	virtual ~patProgram();
#if 0
	std::pair<uint16_t, uint16_t> getPair();
#endif
};

}

}

#endif /* _PAT_H__ */
