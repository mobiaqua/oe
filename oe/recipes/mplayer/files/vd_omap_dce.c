/*
 * OMAP DCE hardware decoding for following video codecs:
 * h264, MPEG4, DivX 5, XviD, MPEG1, MPEG2, WMV9, VC-1
 *
 * Copyright (C) 2020 Pawel Kolodziejski
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config.h"

#include "mp_msg.h"
#include "help_mp.h"

#include "vd_internal.h"
#include "dec_video.h"
#include "../mp_core.h"
#include "../libmpdemux/parse_es.h"
#include "../libmpdemux/mpeg_hdr.h"
#include "../libvo/video_out.h"
#include "../libvo/vo_omap_drm_egl.h"
#include "../ffmpeg/libavcodec/avcodec.h"

static const vd_info_t info = {
	"OMAP DCE codecs decoder",
	"omap_dce",
	"",
	"",
	""
};

LIBVD_EXTERN(omap_dce)

static int init(sh_video_t *sh) {
	return 0;
}

static void uninit(sh_video_t *sh) {
}

static int control(sh_video_t *sh, int cmd, void *arg, ...) {
	return CONTROL_UNKNOWN;
}
static mp_image_t *decode(sh_video_t *sh, void *data, int len, int flags) {
	return NULL;
}
