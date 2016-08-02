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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  42110-1301  USA
 *
 *****************************************************************************/

#ifndef _SDT_H__
#define _SDT_H__

#include "table.h"

#ifndef _DVBPSI_SDT_H_
#include "dvbpsi/sdt.h"
#endif

#include "decode.h"

namespace dvbtee {

namespace decode {

/* SDT (Table) */

class sdt: public Table/*<dvbpsi_sdt_t>*/ {
TABLE_DECODER_TPL
public:
	sdt(Decoder *, TableWatcher*);
	sdt(Decoder *, TableWatcher*, const dvbpsi_sdt_t * const);
	virtual ~sdt();

	void store(const dvbpsi_sdt_t * const);

	static bool ingest(TableStore *s, const dvbpsi_sdt_t * const t, TableWatcher *w = NULL);

	const decoded_sdt_t& getDecodedSDT() const { return decoded_sdt; }

	inline const unsigned int& get_services_w_eit_pf()
	{
		return services_w_eit_pf;
	}
	inline const unsigned int& get_services_w_eit_sched()
	{
		return services_w_eit_sched;
	}

private:
	decoded_sdt_t decoded_sdt;

	unsigned int services_w_eit_pf;
	unsigned int services_w_eit_sched;
};

class sdtSVC: public TableDataComponent {
public:
	sdtSVC(decoded_sdt_service_t&, Decoder*, const dvbpsi_sdt_service_t * const);
	virtual ~sdtSVC();
};

}

}

#endif /* _SDT_H__ */
