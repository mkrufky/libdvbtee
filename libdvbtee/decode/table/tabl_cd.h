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

#ifndef _STT_H__
#define _STT_H__

#include "table.h"

#include "dvbpsi/atsc_stt.h"

namespace dvbtee {

namespace decode {

/* System Time Table (ATSC) */

class stt: public Table/*<dvbpsi_atsc_stt_t>*/ {
public:
	stt(Decoder *);
	stt(Decoder *, TableWatcher*);
	stt(Decoder *, TableWatcher*, dvbpsi_atsc_stt_t*);
	stt(Decoder *, dvbpsi_atsc_stt_t*);
	virtual ~stt();

	const time_t& getTime() const { return m_time; }

	void store(dvbpsi_atsc_stt_t*);

	static bool ingest(TableStore *s, dvbpsi_atsc_stt_t *t, TableWatcher *w = NULL);
private:
	time_t m_time;
};

}

}

#endif /* _STT_H__ */
