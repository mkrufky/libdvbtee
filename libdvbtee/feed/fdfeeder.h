#ifndef FDFEEDER_H
#define FDFEEDER_H

#include "feeder.h"

namespace dvbtee {

namespace feed {

class FdFeeder : public ThreadFeeder
{
public:
	FdFeeder();
	virtual ~FdFeeder();

	virtual void stop();

	void closeFd();

	int openFile(int new_fd) { m_fd = new_fd; return m_fd; } /* assumes already open */

protected:
	int m_fd;
};

}

}

#endif // FDFEEDER_H
