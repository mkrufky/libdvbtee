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

#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include <assert.h>
#include <map>
#include <pthread.h>
#include <vector>

#include "decoder.h"
#include "dvbpsi/descriptor.h"

#include "log.h"

#define desc_check_tag(t, e) \
({ \
	bool __ret = (t == e); \
	if (!__ret) \
		__dprintf(DBG_DESC, "FAIL: 0x%02x != 0x%02x", t, e); \
	__ret; \
})

#define desc_dr_failed(dr) \
({ \
	bool __ret = !dr; \
	if (__ret) dprintf("decoder failed!"); \
	__ret; \
})


namespace dvbtee {

namespace decode {

class Descriptor: public Decoder {
public:
	bool isValid() { return m_valid; }
	uint8_t getTag() { return m_tag; }

protected:
	Descriptor(Decoder *, dvbpsi_descriptor_t*);
	virtual ~Descriptor();

	void setValid(bool v) { m_valid = v; }

	uint8_t m_tag;
private:
	bool m_valid;
};

class DescriptorStore {
public:
	DescriptorStore(Decoder *);
	~DescriptorStore();

	bool add(dvbpsi_descriptor_t*);
	void decode(dvbpsi_descriptor_t*);

	std::vector<Descriptor*> get(uint8_t);

private:
	Decoder *m_parent;
	std::multimap<uint8_t, Descriptor*> m_store;
};

class DescriptorBaseFactory {
public:
	virtual Descriptor* create(Decoder *, dvbpsi_descriptor_t*) = 0;
protected:
	DescriptorBaseFactory() {}
	~DescriptorBaseFactory() {}
};

class DescriptorRegistry {
public:
	static DescriptorRegistry& instance();

	bool registerFactory(uint8_t, DescriptorBaseFactory*);
	DescriptorBaseFactory *getFactory(uint8_t);
	DescriptorBaseFactory *getFactory(dvbpsi_descriptor_t*);

	Descriptor *create(Decoder*, dvbpsi_descriptor_t*);
private:
	DescriptorRegistry();
	~DescriptorRegistry();

	pthread_mutex_t m_mutex;
	std::map <uint8_t, DescriptorBaseFactory*> m_factories;
};

template <class T>
class LinkedDescriptor : public T {
public:
	LinkedDescriptor(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
	 : T(parent, p_descriptor)
	 , m_linkedIdx(-1)
	{
		if (parent) m_linkedIdx = parent->linkChild(this);
	}

	~LinkedDescriptor()
	{
		Decoder *parent = T::getParent();
		if (parent) parent->unlinkChild(m_linkedIdx);
	}

	virtual int getMapIndex() { return m_linkedIdx; }

private:
	int m_linkedIdx;
};

template <uint8_t TAG, class T>
class DescriptorFactory : public DescriptorBaseFactory {
public:
	static DescriptorFactory<TAG, T>& instance()
	{
		static DescriptorFactory<TAG, T> INSTANCE;
		return INSTANCE;
	}
	virtual T *create(Decoder *parent, dvbpsi_descriptor_t *p_descriptor) { return new LinkedDescriptor<T>(parent, p_descriptor); }
private:
	DescriptorFactory() {
		bool descriptorFactoryRegistration = DescriptorRegistry::instance().registerFactory(TAG, this);
		assert(descriptorFactoryRegistration);
	}
	~DescriptorFactory() {}
};

#define REGISTER_DESCRIPTOR_FACTORY(tag, decoder) static DescriptorFactory<tag, decoder> &__DescFactory = DescriptorFactory<tag, decoder>::instance()

}

}

#endif /* __DESCRIPTOR_H__ */
