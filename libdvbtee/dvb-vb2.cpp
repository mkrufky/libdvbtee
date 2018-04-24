/*
 * Copyright (c)      2018 - Michael Ira Krufky <mkrufky@linuxtv.org>
 * Copyright (c) 2017-2018 - Mauro Carvalho Chehab
 * Copyright (c) 2017-2018 - Junghak Sung <jh1009.sung@samsung.com>
 * Copyright (c) 2017-2018 - Satendra Singh Thakur <satendra.t@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/******************************************************************************
 * Implements videobuf2 streaming APIs for DVB
 *****************************************************************************/

#include "dvbtee_config.h"
#ifdef USE_LINUXTV
#include "dvb-vb2.h"
#include "log.h"
#define CLASS_MODULE "dvbvb2"

#define dPrintf(fmt, arg...) __dPrintf(DBG_TUNE, fmt, ##arg)

#define dvb_perror perror
#define dvb_loginfo dPrintf
#define dvb_logerr dPrintf

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>

#include <config.h>

#include <sys/mman.h>

/*Sleep time for retry, in case ioctl fails*/
#define SLEEP_US	1000

#define memzero(x) memset(&(x), 0, sizeof(x))

static inline int xioctl(int fd, unsigned long int cmd, void *arg)
{
	int ret;
	struct timespec stime, etime;
	long long etimell = 0;
	clock_gettime(CLOCK_MONOTONIC, &stime);
	long long stimell = (long long) (stime.tv_sec + 1 /*1 sec wait*/)
					* 1000000000 + stime.tv_nsec;
	do {
		ret = ioctl(fd, cmd, arg);
		if (ret < 0 && (errno == EINTR || errno == EAGAIN)) {
			clock_gettime(CLOCK_MONOTONIC, &etime);
			etimell = (long long) etime.tv_sec * 1000000000 +
					etime.tv_nsec;
			if (etimell > stimell)
				break;
			/* wait for some time to prevent cpu hogging */
			usleep(SLEEP_US);
			continue;
		}
		else
			break;
	} while (1);

	return ret;
}

int stream_qbuf(struct stream_ctx *sc, int idx)
{
	struct dmx_buffer buf;
	int ret;

	memzero(buf);
	buf.index = idx;

	ret = xioctl(sc->in_fd, DMX_QBUF, &buf);
	if (ret < 0) {
		dvb_perror("DMX_QBUF failed");
		return ret;
	}

	return ret;
}

int stream_dqbuf(struct stream_ctx *sc,
		 struct dmx_buffer *buf)
{
	static int64_t count = -1;
#ifdef DVB_VB2_LOG_FLAGS
	int ret, flags;
#else
	int ret;
#endif

	ret = xioctl(sc->in_fd, DMX_DQBUF, buf);
	if (ret < 0) {
		dvb_perror("DMX_DQBUF failed");
		return ret;
	}

	if (count >= 0 && (buf->count != (uint32_t)(count + 1)))
		dvb_logerr("Frame lost. Expected %d, received %d", (uint32_t) count + 1, buf->count);

	count = buf->count;

#ifdef DVB_VB2_LOG_FLAGS
	if (!buf->flags)
		return ret;

	flags = buf->flags;

	if (flags & DMX_BUFFER_FLAG_HAD_CRC32_DISCARD) {
		flags &= ~DMX_BUFFER_FLAG_HAD_CRC32_DISCARD;
		dvb_logerr("Kernel discarded packets with invalid checksums");
	}
	if (flags & DMX_BUFFER_FLAG_TEI) {
		flags &= ~DMX_BUFFER_FLAG_TEI;
		dvb_logerr("Transport Error indicator");
	}
	if (flags & DMX_BUFFER_PKT_COUNTER_MISMATCH) {
		flags &= ~DMX_BUFFER_PKT_COUNTER_MISMATCH;
		dvb_logerr("Packet counter mismatch");
	}
	if (flags & DMX_BUFFER_FLAG_DISCONTINUITY_DETECTED) {
		flags &= ~DMX_BUFFER_FLAG_DISCONTINUITY_DETECTED;
		dvb_logerr("Kernel detected packet discontinuity");
	}
	if (flags & DMX_BUFFER_FLAG_DISCONTINUITY_INDICATOR) {
		flags &= ~DMX_BUFFER_FLAG_DISCONTINUITY_INDICATOR;
		dvb_logerr("Kernel received a discontinuity indicator");
	}
	if (flags)
		dvb_logerr("Unknown error: 0x%04x", flags);
#endif

	return ret;
}

int stream_init(struct stream_ctx *sc,
		int in_fd, int buf_cnt, int buf_size)
{
	struct dmx_requestbuffers req;
	struct dmx_buffer buf;
	int ret;
	int i;

	memset(sc, 0, sizeof(struct stream_ctx));
	sc->in_fd = in_fd;
	sc->buf_size = buf_size;
	sc->buf_cnt = buf_cnt;

	memzero(req);
	req.count = sc->buf_cnt;
	req.size = sc->buf_size;

	ret = xioctl(in_fd, DMX_REQBUFS, &req);
	if (ret) {
		//dvb_perror("DMX_REQBUFS failed");
		return ret;
	}

	if (sc->buf_cnt != req.count) {
		dvb_logerr("buf_cnt %d -> %d changed !!!",
			   sc->buf_cnt, req.count);
		sc->buf_cnt = req.count;
	}

	for (i = 0; i < sc->buf_cnt; i++) {
		memzero(buf);
		buf.index = i;

		ret = xioctl(in_fd, DMX_QUERYBUF, &buf);
		if (ret) {
			dvb_perror("DMX_QUERYBUF failed");
			return ret;
		}

		sc->buf[i] = (unsigned char*)
		            mmap(NULL, buf.length,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					in_fd, buf.offset);

		if (sc->buf[i] == MAP_FAILED) {
			dvb_perror("Failed to MMAP buffer");
			return -1;
		}
		/**enqueue the buffers*/
		ret = stream_qbuf(sc, i);
		if (ret) {
			dvb_perror("stream_qbuf failed");
			return ret;
		}

		sc->buf_flag[i] = 1;
	}

	return 0;
}

void stream_deinit(struct stream_ctx *sc)
{
	struct dmx_buffer buf;
	int ret;
	int i;

	for (i = 0; i < sc->buf_cnt; i++) {
		memzero(buf);
		buf.index = i;

		if (sc->buf_flag[i]) {
			ret = stream_dqbuf(sc, &buf);
			if (ret) {
				dvb_perror("stream_dqbuf failed");
			}
		}
		ret = munmap(sc->buf[i], sc->buf_size);
		if (ret) {
			dvb_perror("munmap failed");
		}

	}

	return;
}
#endif
