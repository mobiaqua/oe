/*
 * video output for OMAP DRM EGL
 *
 * Copyright (C) 2020 Pawel Kolodziejski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>

#include "config.h"
#include "aspect.h"
#include "video_out.h"
#include "video_out_internal.h"
#include "sub/sub.h"
#include "sub/eosd.h"
#include "../mp_core.h"
#include "vo_omap_drm_egl.h"
#include "libavcodec/avcodec.h"

static const vo_info_t info = {
	"omap drm egl video driver",
	"omap_drm_egl",
	"",
	""
};

LIBVO_EXTERN(omap_drm_egl)

static int preinit(const char *arg) {
	return -1;
}

static int config(uint32_t width, uint32_t height, uint32_t d_width, uint32_t d_height, uint32_t flags, char *title, uint32_t format) {
	return -1;
}

static int query_format(uint32_t format) {
	return 0;
}

static uint32_t get_image(mp_image_t *mpi) {
	return VO_FALSE;
}

static uint32_t put_image(mp_image_t *mpi) {
	return VO_NOTIMPL;
}

static int draw_frame(uint8_t *src[]) {
	return VO_FALSE;
}

static int draw_slice(uint8_t *image[], int stride[], int w, int h, int x, int y) {
	return VO_FALSE;
}

static void draw_osd(void) {
}

static void flip_page() {
}

static void check_events(void) {
}

static void uninit() {
}

static int control(uint32_t request, void *data) {
	return VO_NOTIMPL;
}
