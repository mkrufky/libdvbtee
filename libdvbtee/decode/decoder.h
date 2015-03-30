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

#ifndef __DECODER_H__
#define __DECODER_H__

#include <map>
#include <string>
#if LOCK_DECODER_CHILDREN
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdint.h>

#include "dvbpsi/dvbpsi.h"

#include "value.h"
#include "array.h"
#include "object.h"

namespace dvbtee {

namespace decode {

class Decoder: public Object {
public:
	Decoder();
	Decoder(Decoder*, std::string&);
	virtual ~Decoder();

	inline bool isValid() const { return m_valid; }
	const std::string& getDecoderName() const;

	int linkChild(Decoder *d);
	bool unlinkChild(int);

	void showChildren() const;

protected:
	virtual const int getMapIndex() const = 0;
	Decoder *getParent() { return m_parent; }
	void setValid(bool v) { m_valid = v; }

private:
	Decoder *m_parent;
	const std::string m_name;
	bool m_valid;
#if LOCK_DECODER_CHILDREN
	pthread_mutex_t m_mutex;
#endif
	std::map<int, Decoder*> m_children;

	int linkChild(int, Decoder*);
	int __genMapIdx;
	int genMapIndex();
};

class LinkedDecoder: public Decoder {
public:
	LinkedDecoder(Decoder*, std::string&);
	virtual ~LinkedDecoder();

protected:
	virtual const int getMapIndex() const;

private:
	int m_linkedIdx;

	void linkParent();
};

class NullDecoder: public Decoder {
public:
	NullDecoder();
	NullDecoder(Decoder*, std::string&);
	virtual ~NullDecoder();

protected:
	virtual const int getMapIndex() const;
};

}

}

#endif /* __DECODER_H__ */
