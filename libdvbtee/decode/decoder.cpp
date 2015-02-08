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
#if LOCK_DECODER_CHILDREN
	pthread_mutex_init(&m_mutex, 0);
#endif
}

Decoder::Decoder(Decoder *parent)
 : m_parent(parent)
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_init(&m_mutex, 0);
#endif
}

Decoder::~Decoder()
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_destroy(&m_mutex);
#endif
}

int Decoder::registerChild(Decoder *d)
{
	return registerChild(d->getMapIndex(), d);
}

int Decoder::unregisterChild(int idx)
{
	if (!m_children.count(idx)) return -1;
#if LOCK_DECODER_CHILDREN
	pthread_mutex_lock(&m_mutex);
#endif
	m_children.erase(idx);
#if LOCK_DECODER_CHILDREN
	pthread_mutex_unlock(&m_mutex);
#endif
}

int Decoder::registerChild(int idx, Decoder *d)
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_lock(&m_mutex);
#endif

	if (idx < 0)
	    idx = m_children.size();

	if (m_children.count(idx)) {
#if LOCK_DECODER_CHILDREN
		pthread_mutex_unlock(&m_mutex);
#endif
		return -1;
	}

	m_children.insert( std::pair<unsigned int, Decoder*>(idx, d) );

#if LOCK_DECODER_CHILDREN
	pthread_mutex_unlock(&m_mutex);
#endif

	return idx;

}
