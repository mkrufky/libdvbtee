#include "fdfeeder.h"

#include "log.h"
#define CLASS_MODULE "FdFeeder"

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

FdFeeder::FdFeeder()
  : m_fd(-1)
{
	//
}

FdFeeder::~FdFeeder()
{
	closeFd();
}

void FdFeeder::stop()
{
	dPrintf("()");

	stop_without_wait();

	dPrintf("waiting...");

	while (-1 != m_fd) {
		usleep(20*1000);
	}

	dPrintf("done");
}

void FdFeeder::closeFd()
{
	dPrintf("()");

	if (m_fd >= 0) {
		close(m_fd);
		m_fd = -1;
	}
}

int FdFeeder::openFile(int new_fd)
{
	closeFd();
	m_fd = new_fd;
	return m_fd;
}
