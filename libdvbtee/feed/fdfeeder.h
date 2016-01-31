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

	/* assumes already open */
	int openFile(int new_fd);

protected:
	int m_fd;
};

}

}

#endif // FDFEEDER_H
