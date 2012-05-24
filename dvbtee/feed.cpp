#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "feed.h"

feed::feed()
  : f_kill_thread(false)
  , fd(-1)
{
	fprintf(stderr, "%s()\n", __func__);

	memset(filename, 0, sizeof(filename));
}

feed::~feed()
{
	fprintf(stderr, "%s()\n", __func__);

	close_file();
}

void feed::set_filename(char* new_file)
{
	fprintf(stderr, "%s(%s)\n", __func__, new_file);

	strcpy(filename, new_file);
};

int feed::open_file()
{
	fprintf(stderr, "%s()\n", __func__);

	fd = -1;

	if ((fd = open(filename, O_RDONLY | O_NONBLOCK )) < 0)
		fprintf(stderr, "failed to open %s\n", filename);
	else
		fprintf(stderr, "%s: using %s\n", __func__, filename);

	return fd;
}

void feed::close_file()
{
	fprintf(stderr, "%s()\n", __func__);

	close(fd);
	fd = -1;
}

//static
void* feed::feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->feed_thread();
}

//static
void* feed::stdin_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->stdin_feed_thread();
}

int feed::start()
{
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

void feed::stop()
{
	fprintf(stderr, "%s()\n", __func__);

	stop_without_wait();

	while (-1 != fd) {
		usleep(20*1000);
	}
}

#define BUFSIZE (188 * 312)
void *feed::feed_thread()
{
	fprintf(stderr, "%s(fd=%d)\n", __func__, fd);

	while (!f_kill_thread) {
		unsigned char buf[BUFSIZE];
		unsigned char *p_buf = buf;
		ssize_t r;

		if ((r = read(fd, buf, BUFSIZE)) <= 0) {

			if (!r) {
				f_kill_thread = true;
				continue;
			}
			// FIXME: handle (r < 0) errror cases
			//if (ret <= 0) switch (errno) {
			switch (errno) {
			case EAGAIN:
				break;
			case EOVERFLOW:
				fprintf(stderr, "%s: r = %d, errno = EOVERFLOW\n", __func__, (int)r);
				break;
			case EBADF:
				fprintf(stderr, "%s: r = %d, errno = EBADF\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EFAULT:
				fprintf(stderr, "%s: r = %d, errno = EFAULT\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EINTR: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EINTR\n", __func__, (int)r);
				//f_kill_thread = true;
				break;
			case EINVAL:
				fprintf(stderr, "%s: r = %d, errno = EINVAL\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EIO: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EIO\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EISDIR:
				fprintf(stderr, "%s: r = %d, errno = EISDIR\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			default:
				fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
				break;
			}

			usleep(50*1000);
			continue;
		}
		parser.feed(r, p_buf);
	}
	close_file();
	pthread_exit(NULL);
}

void *feed::stdin_feed_thread()
{
	fprintf(stderr, "%s()\n", __func__);

	while (!f_kill_thread) {
		unsigned char buf[BUFSIZE];
		unsigned char *p_buf = buf;
		ssize_t r;

		if ((r = fread(buf, 188, BUFSIZE / 188, stdin)) < (BUFSIZE / 188)) {
			if (ferror(stdin)) {
				fprintf(stderr, "%s: error reading stdin!\n", __func__);
				usleep(50*1000);
				clearerr(stdin);
			}
			if (feof(stdin)) {
				fprintf(stderr, "%s: EOF\n", __func__);
				f_kill_thread = true;
			}
			continue;
		}
		parser.feed(r * 188, p_buf);
	}
	pthread_exit(NULL);
}

int feed::start_stdin()
{
	fprintf(stderr, "%s()\n", __func__);

	if (NULL == freopen(NULL, "rb", stdin)) {
		fprintf(stderr, "failed to open stdin!\n");
		return -1;
	}
	fprintf(stderr, "%s: using STDIN\n", __func__);

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, stdin_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}
