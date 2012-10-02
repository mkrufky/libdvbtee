/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef __RBUF_H__
#define __RBUF_H__

#include <pthread.h>
#include <string.h>

class rbuf {
public:
    rbuf();
    ~rbuf();

    void set_capacity(int);
    int  get_capacity();
    int  get_size();
    void dealloc();

    void reset();
    bool check();

    int  get_write_ptr(void**);
    void put_write_ptr(int);
    bool write(const void*, int);

    int  get_read_ptr(void**, int);
    void put_read_ptr(int);
    int  read(void*, int);

private:
    pthread_mutex_t mutex;

    int capacity;
    char* p_data;

    int idx_read;
    int idx_write;

    int  __get_size();
    void __reset();

    int  __get_write_ptr(void**);
    void __put_write_ptr(int);

    int  __get_read_ptr(void**, int);
    void __put_read_ptr(int);
};

#endif /* __RBUF_H__ */
