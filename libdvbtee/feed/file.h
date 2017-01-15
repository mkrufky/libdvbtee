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

#ifndef FILE_H
#define FILE_H

#include "fdfeeder.h"

namespace dvbtee {

namespace feed {

class FileFeeder : public FdFeeder
{
public:
	FileFeeder();
	virtual ~FileFeeder();

	virtual int start();

	virtual
	int openFile(char* new_file, int flags = 0) { setFilename(new_file); return doOpenFile(flags); }

private:
	void        *file_feed_thread();
	static void *file_feed_thread(void*);

	void setFilename(char* newFile);
	int doOpenFile(int flags);
};

}

}

#endif // FILE_H
