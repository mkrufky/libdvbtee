/*****************************************************************************
 * Copyright (C) 2013 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "hlsinput.h"

int main(int argc, char *argv[])
{
	int opt;
	bool feed_stdout = false;
	char url[2048] = { 0 };

	while ((opt = getopt(argc, argv, "i:o")) != -1) {
		switch (opt) {
		case 'i':
			if (!optarg) {
				fprintf(stderr, "missing argument\n");
				return -1;
			}
			strncpy(url, optarg, sizeof(url));
			break;
		case 'o':
			feed_stdout = true;
			break;
		}
	}

	hlsinput hls(feed_stdout);
	hls.get(url);
}
