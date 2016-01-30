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

#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include <assert.h>
#include <map>
#include <pthread.h>
#include <vector>

#include "decode/decoder.h"
#include "dvbpsi/descriptor.h"

#include "log.h"

#define desc_check_tag(t, e) \
({ \
	bool __ret = (t == e); \
	if (!__ret) \
		__dPrintf(DBG_DESC, "FAIL: 0x%02x != 0x%02x", t, e); \
	__ret; \
})

#define desc_dr_failed(dr) \
({ \
	bool __ret = !dr; \
	if (__ret) dPrintf("decoder failed!"); \
	__ret; \
})


namespace dvbtee {

namespace decode {

class Descriptor: public Decoder {
public:
	const uint8_t& getTag() const;

protected:
	Descriptor(Decoder *, std::string&, dvbpsi_descriptor_t*);
	virtual ~Descriptor();

private:
	uint8_t m_tag;

	void init();
};

class DescriptorStore : public valueobj::Array {
public:
#if 1 // temporarily allowing NULL parent during refactoring
	DescriptorStore(Decoder *parent = NULL);
#else
	DescriptorStore(Decoder *);
#endif
	~DescriptorStore();

	bool add(dvbpsi_descriptor_t*);
	void decode(dvbpsi_descriptor_t*);

	const std::vector<Descriptor*> get(uint8_t) const;
	const Descriptor *last(uint8_t) const;

private:
	Decoder *m_parent;
	std::multimap<uint8_t, Descriptor*> m_store;
};

class DescriptorBaseFactory {
public:
	virtual Descriptor* create(Decoder *, dvbpsi_descriptor_t*) const = 0;
protected:
	DescriptorBaseFactory() {}
	~DescriptorBaseFactory() {}
};

class DescriptorRegistry {
public:
	static DescriptorRegistry& instance();

	bool registerFactory(uint8_t, DescriptorBaseFactory*);
	const DescriptorBaseFactory *getFactory(uint8_t) const;
	const DescriptorBaseFactory *getFactory(dvbpsi_descriptor_t*) const;

	Descriptor *create(Decoder*, dvbpsi_descriptor_t*);

	int count() const;
private:
	DescriptorRegistry();
	~DescriptorRegistry();

	mutable pthread_mutex_t m_mutex;
	std::map <uint8_t, const DescriptorBaseFactory*> m_factories;
};

template <class T>
class LinkedDescriptor : public T {
public:
	LinkedDescriptor(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
	 : T(parent, p_descriptor)
	 , m_linkedIdx(-1)
	{
		linkParent(parent);
	}

	~LinkedDescriptor()
	{
		Decoder *parent = T::getParent();
		if (parent) parent->unlinkChild(m_linkedIdx);
	}

	virtual int getMapIndex() const
	{
		return m_linkedIdx;
	}

private:
	int m_linkedIdx;

	inline void linkParent(Decoder *parent)
	{
		if (parent) m_linkedIdx = parent->linkChild(this);
	}
};

template <uint8_t TAG, class T>
class DescriptorFactory : public DescriptorBaseFactory {
public:
	static const DescriptorFactory<TAG, T>& instance()
	{
		static DescriptorFactory<TAG, T> INSTANCE;
		return INSTANCE;
	}
	virtual T *create(Decoder *parent, dvbpsi_descriptor_t *p_descriptor) const
	{
		return new LinkedDescriptor<T>(parent, p_descriptor);
	}
private:
	DescriptorFactory() {
		bool descriptorFactoryRegistration = DescriptorRegistry::instance().registerFactory(TAG, this);
		assert(descriptorFactoryRegistration);
	}
	~DescriptorFactory() {}
};

#define DESCRIPTOR_DECODER_TPL \
	public:  static void a(); \

#define REGISTER_DESCRIPTOR_FACTORY(tag, decoder) \
	\
	static volatile const DescriptorFactory<tag, decoder> &__DescFactory = DescriptorFactory<tag, decoder>::instance();\
	\
	void decoder::a() { __DescFactory.instance(); } \

}

}

#endif /* __DESCRIPTOR_H__ */
