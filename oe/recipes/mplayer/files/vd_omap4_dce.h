/*
 * OMAP 4 ducati hardware decoding
 *
 * Copyright (C) 2011 Pawel Kolodziejski
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef MPLAYER_VD_OMAP4_DCE_H
#define MPLAYER_VD_OMAP4_DCE_H

struct v4l2_buf {
	struct v4l2_buffer buffer;
	unsigned char *plane[3];
	unsigned char *plane_p[3];
	int used;
	int to_free;
	int locked;
	int interlaced;
};

struct omap4_dce_priv {
	int codec_id;
	int (*reset_buffers)(void);
};

// FIXME: hack
extern struct omap4_dce_priv omap4_dce_priv_t;

#define ALIGN2(value, align) (((value) + ((1 << (align)) - 1)) & ~((1 << (align)) - 1))

#endif
