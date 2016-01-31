#ifndef FDFEEDER_H
#define FDFEEDER_H

#include "feeder.h"

namespace dvbtee {

namespace feed {

class UriFeeder
{
public:
	UriFeeder();
	virtual ~UriFeeder();

protected:
	char m_uri[256];
};

class FdFeeder : public ThreadFeeder, public UriFeeder
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
