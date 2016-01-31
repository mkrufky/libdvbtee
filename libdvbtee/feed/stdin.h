#ifndef STDINFEEDER_H
#define STDINFEEDER_H

#include "fdfeeder.h"

namespace dvbtee {

namespace feed {

class StdinFeeder : public ThreadFeeder, public UriFeeder
{
public:
	StdinFeeder();
	virtual ~StdinFeeder();

	virtual int start();

private:
	void        *stdin_feed_thread();
	static void *stdin_feed_thread(void*);
};

}

}

#endif // STDINFEEDER_H
