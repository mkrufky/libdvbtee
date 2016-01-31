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

#ifndef FDFEEDER_H
#define FDFEEDER_H

#include "feeder.h"

namespace dvbtee {

namespace feed {

class UriFeeder
{
public:
	UriFeeder();
	virtual ~UriFeeder();

	std::string getUri() { return std::string(m_uri); }

protected:
	char m_uri[256];
};

class FdFeeder : public ThreadFeeder, public UriFeeder
{
public:
	FdFeeder();
	virtual ~FdFeeder();

	virtual void stop();

	void closeFd();

	/* assumes already open */
	int openFile(int new_fd);

protected:
	int m_fd;
};

}

}

#endif // FDFEEDER_H
