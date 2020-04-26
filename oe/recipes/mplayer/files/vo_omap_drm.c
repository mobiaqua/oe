/*
 * video output for OMAP DRM
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
#include <sys/mman.h>

#include "config.h"
#include "aspect.h"

#include "video_out.h"
#include "video_out_internal.h"
#include "sub/sub.h"
#include "sub/eosd.h"
#include "../mp_core.h"
#include "osdep/timer.h"
#include "libavcodec/avcodec.h"

#include <libdrm/omap_drmif.h>
#include <libswscale/swscale.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

static struct frame_info {
    unsigned int w;
    unsigned int h;
    unsigned int dx;
    unsigned int dy;
    unsigned int dw;
    unsigned int dh;
    unsigned int y_stride;
    unsigned int uv_stride;
} yuv420_frame_info, nv12_frame_info;

int yuv420_to_nv12_convert(unsigned char *vdst[3], unsigned char *vsrc[3], unsigned char *, unsigned char *);
void yuv420_to_nv12_open(struct frame_info *dst, struct frame_info *src);

#define ALIGN2(value, align) (((value) + ((1 << (align)) - 1)) & ~((1 << (align)) - 1))

static const vo_info_t info = {
	"omap drm video driver",
	"omap_drm",
	"",
	""
};

typedef struct {
	int             fd;
	struct gbm_bo   *gbmBo;
	uint32_t        fbId;
} DrmFb;

typedef struct {
	void           *priv;
	struct omap_bo *bo;
	uint32_t       boHandle;
	int            locked;
} DisplayVideoBuffer;

typedef struct {
	int     handle;
} DisplayHandle;

typedef struct {
	DisplayHandle handle;
	int (*getVideoBuffer)(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height);
	int (*releaseVideoBuffer)(DisplayVideoBuffer *handle);
} omap_drm_priv_t;

extern omap_drm_priv_t omap_drm_priv;

static int                         _dce;
static int                         _initialized;
static int                         _fd;
static struct omap_device          *_omapDevice;

static drmModeResPtr               _drmResources;
static drmModePlaneResPtr          _drmPlaneResources;
static drmModeCrtcPtr              _oldCrtc;
static drmModeModeInfo             _modeInfo;
static uint32_t                    _connectorId;
static uint32_t                    _crtcId;
static int                         _planeId;

static struct SwsContext           *_scaleCtx;
static uint32_t                    _fbWidth, _fbHeight;
static int                         _dstWidth, _dstHeight;
static struct omap_bo              *_primaryFbBo;
static uint32_t                    _primaryFbId;

LIBVO_EXTERN(omap_drm)

static int getVideoBuffer(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height);
static int releaseVideoBuffer(DisplayVideoBuffer *handle);

static int preinit(const char *arg) {
	return -1;
}

static void uninit(void) {
}

static int getHandle(DisplayHandle *handle) {
}

static int getVideoBuffer(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height) {
	return -1;
}

static int releaseVideoBuffer(DisplayVideoBuffer *handle) {
	return 0;
}

static int config(uint32_t width, uint32_t height, uint32_t d_width, uint32_t d_height, uint32_t flags, char *title, uint32_t format) {
	if (!_initialized)
		return -1;

	switch (format) {
	case IMGFMT_NV12:
		_dce = 1;
		break;
	case IMGFMT_YV12:
		_dce = 0;
		break;
	default:
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm] config() Error wrong pixel format\n");
		return -1;
	}

	_dstWidth = d_width;
	_dstHeight = d_width;

	return 0;
}

static int query_format(uint32_t format) {
	if (format == IMGFMT_YV12 || format == IMGFMT_NV12)
		return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_EOSD | VFCAP_EOSD_UNSCALED |
		       VFCAP_FLIP | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN | VOCAP_NOSLICES;

	return 0;
}

static uint32_t get_image(mp_image_t *mpi) {
	if (!_dce) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm] Error: get_image() only for hardware decoding\n");
		return VO_NOTIMPL;
	}
	if (mpi->type == MP_IMGTYPE_TEMP) {
		mpi->flags |= MP_IMGFLAG_DIRECT | MP_IMGFLAG_DRAW_CALLBACK;
		return VO_TRUE;
	} else {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm] Error: get_image() only for MP_IMGTYPE_TEMP\n");
		return VO_FALSE;
	}
}

static int draw_frame(uint8_t *src[]) {
	// empty
	return VO_FALSE;
}

static int draw_slice(uint8_t *image[], int stride[], int w,int h,int x,int y) {
	// empty
	return VO_FALSE;
}

static uint32_t put_image(mp_image_t *mpi) {
	return VO_FALSE;
}

static void draw_osd(void) {
	// todo
}

static void flip_page() {
}

static void check_events(void) {
	// empty
}

static int control(uint32_t request, void *data) {
	switch (request) {
	case VOCTRL_QUERY_FORMAT:
		return query_format(*((uint32_t *)data));
	case VOCTRL_FULLSCREEN:
		return VO_TRUE;
	case VOCTRL_UPDATE_SCREENINFO:
		vo_screenwidth = _fbWidth;
		vo_screenheight = _fbHeight;
		aspect_save_screenres(vo_screenwidth, vo_screenheight);
		return VO_TRUE;
	case VOCTRL_GET_IMAGE:
		return get_image(data);
	case VOCTRL_DRAW_IMAGE:
		return put_image(data);
	case VOCTRL_GET_EOSD_RES: {
			struct mp_eosd_settings *r = data;
			r->mt = r->mb = r->ml = r->mr = 0;
			r->srcw = _fbWidth;
			r->srch = _fbHeight;
			r->w = _fbWidth;
			r->h = _fbHeight;
		}
		// todo
		return VO_TRUE;
	case VOCTRL_DRAW_EOSD:
		if (!data)
			return VO_FALSE;
		// todo
		return VO_TRUE;
	}

	return VO_NOTIMPL;
}
