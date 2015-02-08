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

#include "decoder.h"

using namespace dvbtee::decode;

Decoder::Decoder()
 : m_parent(NULL)
{
	pthread_mutex_init(&m_mutex, 0);
}

Decoder::Decoder(Decoder *parent)
 : m_parent(parent)
{
	pthread_mutex_init(&m_mutex, 0);
}

Decoder::~Decoder()
{
	pthread_mutex_destroy(&m_mutex);
}

int Decoder::registerChild(Decoder *d)
{
	return registerChild(d->getMapIndex(), d);
}

int Decoder::unregisterChild(int idx)
{
	if (!m_children.count(idx)) return -1;
	pthread_mutex_lock(&m_mutex);
	m_children.erase(idx);
	pthread_mutex_unlock(&m_mutex);
}

int Decoder::registerChild(int idx, Decoder *d)
{
	pthread_mutex_lock(&m_mutex);

	if (idx < 0)
	    idx = m_children.size();

	if (m_children.count(idx)) {
		pthread_mutex_unlock(&m_mutex);
		return -1;
	}

	m_children.insert( std::pair<unsigned int, Decoder*>(idx, d) );

	pthread_mutex_unlock(&m_mutex);

	return idx;

}
