#include <errno.h>
#include <fcntl.h>

#include "file.h"

#include "log.h"
#define CLASS_MODULE "FileFeeder"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

#define BUFSIZE ((4096/188)*188)

FileFeeder::FileFeeder()
{
  //
}

FileFeeder::~FileFeeder()
{
  //
}

void FileFeeder::setFilename(char* newFile)
{
	dPrintf("(%s)", newFile);

	size_t len = strlen(newFile);
	strncpy(m_uri, newFile, sizeof(m_uri)-1);
	m_uri[len < sizeof(m_uri) ? len : sizeof(m_uri)-1] = '\0';
}

int FileFeeder::doOpenFile(int flags)
{
	dPrintf("()");

#if !USE_IOS_READ
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

int FileFeeder::start()
{
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, file_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* FileFeeder::file_feed_thread(void *p_this)
{
	return static_cast<FileFeeder*>(p_this)->file_feed_thread();
}

void *FileFeeder::file_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dPrintf("(fd=%d)", m_fd);

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;
		if ((r = read(m_fd, q, available)) <= 0) {

			if (!r) {
				f_kill_thread = true;
				continue;
			}
			switch (errno) {
			case EAGAIN:
				break;
			case EOVERFLOW:
				fprintf(stderr, "%s: r = %d, errno = EOVERFLOW\n", __func__, (int)r);
				break;
			case EBADF:
				fprintf(stderr, "%s: r = %d, errno = EBADF\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EFAULT:
				fprintf(stderr, "%s: r = %d, errno = EFAULT\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EINTR: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EINTR\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EINVAL:
				fprintf(stderr, "%s: r = %d, errno = EINVAL\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EIO: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EIO\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EISDIR:
				fprintf(stderr, "%s: r = %d, errno = EISDIR\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			default:
				fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
				break;
			}
			continue;
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r);
#else
		parser.feed(r, q);
#endif
	}
	closeFd();
	pthread_exit(NULL);
}
