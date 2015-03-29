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

#include "descriptor.h"

#define CLASS_MODULE "DESCRIPTOR"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;


Descriptor::Descriptor(Decoder *parent, std::string &name, dvbpsi_descriptor_t *p_dvbpsi_descriptor)
 : Decoder(parent, name)
 , m_tag(0xff)
{
	if (p_dvbpsi_descriptor) m_tag = p_dvbpsi_descriptor->i_tag;
	init();
}

Descriptor::~Descriptor()
{
	//
}

void Descriptor::init()
{
	set("descriptorTag", m_tag);
}

const uint8_t& Descriptor::getTag() const
{
	return m_tag;
}


bool DescriptorRegistry::registerFactory(uint8_t tag, DescriptorBaseFactory *factory)
{
	pthread_mutex_lock(&m_mutex);

	if (m_factories.count(tag)) {
		pthread_mutex_unlock(&m_mutex);
		return false;
	}

	m_factories.insert( std::pair<uint8_t, DescriptorBaseFactory*>(tag,factory) );

	fprintf(stderr, "inserted 0x%02x, %p, %ld descriptor decoders present\n", tag, factory, m_factories.size());

	pthread_mutex_unlock(&m_mutex);
	return true;
}

DescriptorBaseFactory *DescriptorRegistry::getFactory(uint8_t tag)
{
	pthread_mutex_lock(&m_mutex);

	if (!m_factories.count(tag)) {
		pthread_mutex_unlock(&m_mutex);
		return NULL;
	}

	DescriptorBaseFactory* ret = m_factories[tag];

	pthread_mutex_unlock(&m_mutex);

	return ret;
}

DescriptorBaseFactory *DescriptorRegistry::getFactory(dvbpsi_descriptor_t *d)
{
	return getFactory(d->i_tag);
}

Descriptor *DescriptorRegistry::create(Decoder *parent, dvbpsi_descriptor_t *p_dvbpsi_descriptor)
{
	DescriptorBaseFactory *Factory = getFactory(p_dvbpsi_descriptor);
	if (!Factory)
		return NULL;

	return Factory->create(parent, p_dvbpsi_descriptor);
}

DescriptorRegistry::DescriptorRegistry()
{
	pthread_mutex_init(&m_mutex, 0);
}

DescriptorRegistry::~DescriptorRegistry()
{
	pthread_mutex_destroy(&m_mutex);
}

DescriptorRegistry &DescriptorRegistry::instance()
{
	static DescriptorRegistry INSTANCE;
	return INSTANCE;
}


DescriptorStore::DescriptorStore(Decoder *parent)
 : m_parent(parent)
{
	//
}

DescriptorStore::~DescriptorStore()
{
	//
}

bool DescriptorStore::add(dvbpsi_descriptor_t *p_descriptor)
{
	Descriptor *d = NULL;
	if (p_descriptor) d = DescriptorRegistry::instance().create(m_parent, p_descriptor);
	if (d) {
		m_store.insert( std::pair<uint8_t, Descriptor*>(d->getTag(), d) );
		push((Object*)d);
	}
	return (d != NULL);
}

void DescriptorStore::decode(dvbpsi_descriptor_t *p_descriptor)
{
	while (p_descriptor) {
		if (!add(p_descriptor))
			dprintf("failed to decode descriptor! tag: %02x", p_descriptor->i_tag);
		p_descriptor = p_descriptor->p_next;
	}
}

const std::vector<Descriptor *> DescriptorStore::get(uint8_t tag) const
{
	std::vector<Descriptor *> ret;
	std::pair <std::multimap<uint8_t, Descriptor*>::const_iterator, std::multimap<uint8_t, Descriptor*>::const_iterator> range;

	range = m_store.equal_range(tag);

	for (std::multimap<uint8_t, Descriptor*>::const_iterator it=range.first; it!=range.second; ++it)
		ret.push_back(it->second);

	return ret;
}

Descriptor *DescriptorStore::last(uint8_t tag)
{
	std::vector<Descriptor*> D = get(tag);
	ssize_t s = D.size();
	if (s > 1) fprintf(stderr, "tag: %02x, %ld collected, returning last\n", tag, s);
	if (s) return D[s-1];
	return NULL;
}
