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
using namespace dvbtee::value;

Decoder::Decoder()
 : m_parent(NULL)
 , m_name("dvbtee")
 , m_valid(false)
 , __genMapIdx(0)
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_init(&m_mutex, 0);
#endif
}

Decoder::Decoder(Decoder *parent, std::string &name)
 : m_parent(parent)
 , m_name(name)
 , __genMapIdx(0)
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_init(&m_mutex, 0);
#endif
}

Decoder::~Decoder()
{
	//for (std::map<int, Decoder*>::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
	while (m_children.size()) {
		Decoder *child = m_children.begin()->second;
		m_children.begin()->second = NULL;
		/* child calls unlinkChild on its parent inside its destructor */
		if (child) delete child;
	}
#if LOCK_DECODER_CHILDREN
	pthread_mutex_destroy(&m_mutex);
#endif
}

const std::string& Decoder::getDecoderName() const
{
	return m_name;
}

int Decoder::linkChild(Decoder *d)
{
	return linkChild(d->getMapIndex(), d);
}

bool Decoder::unlinkChild(int idx)
{
	if (!m_children.count(idx)) return false;
#if LOCK_DECODER_CHILDREN
	pthread_mutex_lock(&m_mutex);
#endif
	m_children.erase(idx);
#if LOCK_DECODER_CHILDREN
	pthread_mutex_unlock(&m_mutex);
#endif
	return true;
}

void Decoder::showChildren() const
{
	Array decoders;
	for (std::map<int, Decoder*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
		Decoder *child = it->second;
		if (!child) continue;
		decoders.push((Object*)child);
	}
	fprintf(stderr, "%s: %s\n", __func__, decoders.toJson().c_str());
}

int Decoder::linkChild(int idx, Decoder *d)
{
#if LOCK_DECODER_CHILDREN
	pthread_mutex_lock(&m_mutex);
#endif

	if (idx < 0)
	    idx = genMapIndex();

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

int Decoder::genMapIndex()
{
	int idx = __genMapIdx++;
	if (m_children.count(idx))
		return genMapIndex();
	return idx;
}


LinkedDecoder::LinkedDecoder(Decoder *parent, std::string &name)
 : Decoder(parent, name)
 , m_linkedIdx(-1)
{
	linkParent();
}

LinkedDecoder::~LinkedDecoder()
{
	Decoder *parent = getParent();
	if (parent) parent->unlinkChild(m_linkedIdx);
}

const int LinkedDecoder::getMapIndex() const
{
	return m_linkedIdx;
}

void LinkedDecoder::linkParent()
{
	Decoder *parent = getParent();
	if (parent) m_linkedIdx = parent->linkChild(this);
}


NullDecoder::NullDecoder()
 : Decoder()
{

}

NullDecoder::NullDecoder(Decoder *parent, std::string &name)
 : Decoder(parent, name)
{

}

NullDecoder::~NullDecoder()
{

}

const int NullDecoder::getMapIndex() const
{
	return -1;
}
