#include <errno.h>
#include <fstream>

#include "ifsfeeder.h"

#include "log.h"
#define CLASS_MODULE "IfsFeeder"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

#define BUFSIZE ((4096/188)*188)

IfsFeeder::IfsFeeder()
{
  //
}

IfsFeeder::~IfsFeeder()
{
  //
}

void IfsFeeder::setFilename(char* newFile)
{
	dPrintf("(%s)", newFile);

	size_t len = strlen(newFile);
	strncpy(m_uri, newFile, sizeof(m_uri)-1);
	m_uri[len < sizeof(m_uri) ? len : sizeof(m_uri)-1] = '\0';
}

int IfsFeeder::doOpenFile(int flags)
{
	dPrintf("()");

#if 0
	int fd = -1;

	if ((fd = open(m_uri, O_RDONLY|flags )) < 0)
		fprintf(stderr, "failed to open %s\n", m_uri);
	else {
		FdFeeder::openFile(fd);
		fprintf(stderr, "%s: using %s\n", __func__, m_uri);
	}

	return fd;
#else
	return 0;
#endif
}

int IfsFeeder::start()
{
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, file_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* IfsFeeder::file_feed_thread(void *p_this)
{
	return static_cast<IfsFeeder*>(p_this)->file_feed_thread();
}

void *IfsFeeder::file_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dPrintf("(ios)");
	std::ifstream infile;
	infile.open(m_uri, std::ios::binary | std::ios::in);
	if (infile.fail()) {
		switch (errno) {
		case EACCES:
			fprintf(stderr, "%s: r = %d, errno = EACCES\n", __func__, (int)r);
			break;
		case ENOENT:
			fprintf(stderr, "%s: r = %d, errno = ENOENT\n", __func__, (int)r);
			break;
		default:
			fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
			break;
		}
	} else

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;

		infile.read((char *)q, available);
		r = available;
		if (infile.fail()) {
			r = 0;
			int err = errno;
			switch (err) {
			case EACCES:
				fprintf(stderr, "%s: r = %d, errno = EACCES\n", __func__, (int)r);
				break;
			case ENOENT:
				fprintf(stderr, "%s: r = %d, errno = ENOENT\n", __func__, (int)r);
				break;
			default:
				if (err) fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, err);
				break;
			}
			f_kill_thread = true;
			continue;
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r);
#else
		parser.feed(r, q);
#endif
	}
	//close_file();
	pthread_exit(NULL);
}
