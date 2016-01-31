#include <errno.h>
#include <fstream>

#include "stdin.h"

#include "log.h"
#define CLASS_MODULE "StdinFeeder"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

#define BUFSIZE ((4096/188)*188)

StdinFeeder::StdinFeeder()
{
  //
}

StdinFeeder::~StdinFeeder()
{
  //
}

int StdinFeeder::start()
{
	dPrintf("()");

	if (NULL == freopen(NULL, "rb", stdin)) {
		fprintf(stderr, "failed to open stdin!\n");
		return -1;
	}
	fprintf(stderr, "%s: using STDIN\n", __func__);
	strncpy(m_uri, "STDIN", sizeof(m_uri));

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, stdin_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");
#if FEED_BUFFER
	else
		start_feed();
#endif
	return ret;
}

//static
void* StdinFeeder::stdin_feed_thread(void *p_this)
{
	return static_cast<StdinFeeder*>(p_this)->stdin_feed_thread();
}

void *StdinFeeder::stdin_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dPrintf("()");

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;
		if ((r = fread(q, 188, available / 188, stdin)) < (available / 188)) {
			if (ferror(stdin)) {
				fprintf(stderr, "%s: error reading stdin!\n", __func__);
				usleep(50*1000);
				clearerr(stdin);
			}
			if (feof(stdin)) {
				fprintf(stderr, "%s: EOF\n", __func__);
				f_kill_thread = true;
			}
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r * 188);
#else
		parser.feed(r * 188, q);
#endif
	}
	pthread_exit(NULL);
}
