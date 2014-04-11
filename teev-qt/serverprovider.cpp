/*****************************************************************************
 * Copyright (C) 2013-2014 Michael Ira Krufky
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

#include "serverprovider.h"

void ServerProvider::stop_server()
{
	if (!server)
		return;

	if (server->is_running())
		server->stop();

	delete server;
	server = NULL;

	return;
}

int ServerProvider::start_server(uint16_t port_requested, unsigned int flags)
{
	if (server) return -1;

	server = new serve;

	for (map_tuners::const_iterator iter = tuners.begin(); iter != tuners.end(); ++iter) {
		server->add_tuner(iter->second);

		if (flags & 2)
			iter->second->feeder.parser.out.add_http_server(port_requested+1+iter->first);
	}
	server->set_scan_flags(0, flags >> 2);

	return server->start(port_requested);
}

ServerProvider::ServerProvider()
	: TunerProvider()
	, server(NULL)
{
}

ServerProvider::~ServerProvider()
{
	stop_server();
}
