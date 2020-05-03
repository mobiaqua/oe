/*
 * This file is part of mpv.
 *
 * mpv is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * mpv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mpv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <poll.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <libdrm/omap_drmif.h>
#include <xf86drmMode.h>
#include <libswscale/swscale.h>

#include "common/msg.h"
#include "osdep/timer.h"
#include "sub/osd.h"
#include "video/fmt-conversion.h"
#include "video/mp_image.h"
#include "video/sws_utils.h"
#include "vo.h"

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

typedef struct {
	void           *priv;
	struct omap_bo *bo;
	uint32_t       boHandle;
	int            locked;
} DisplayVideoBuffer;

typedef struct {
	struct omap_bo  *bo;
	uint32_t        fbId;
	void            *ptr;
	uint32_t        width, height;
	uint32_t        stride;
	uint32_t        size;
} OSDBuffer;

typedef struct {
	struct omap_bo  *bo;
	uint32_t        boHandle;
	uint32_t        fbId;
	void            *ptr;
	uint32_t        width, height;
	uint32_t        stride;
	uint32_t        size;
	uint32_t        srcX, srcY;
	uint32_t        srcWidth, srcHeight;
	uint32_t        dstX, dstY;
	uint32_t        dstWidth, dstHeight;
	DisplayVideoBuffer *db;
} VideoBuffer;

typedef struct {
	int     handle;
} DisplayHandle;

typedef struct {
	DisplayHandle handle;
	int (*getDisplayVideoBuffer)(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height);
	int (*releaseDisplayVideoBuffer)(DisplayVideoBuffer *handle);
} omap_dce_share_t;

//extern omap_dce_share_t omap_dce_share;
omap_dce_share_t omap_dce_share;

#define NUM_OSD_FB   2
#define NUM_VIDEO_FB 3

struct priv {
	int                         _dce;
	int                         _initialized;
	int                         _fd;
	struct omap_device          *_omapDevice;

	drmModeResPtr               _drmResources;
	drmModePlaneResPtr          _drmPlaneResources;
	drmModeCrtcPtr              _oldCrtc;
	drmModeModeInfo             _modeInfo;
	uint32_t                    _connectorId;
	uint32_t                    _crtcId;
	int                         _osdPlaneId;
	int                         _videoPlaneId;

	struct omap_bo              *_primaryFbBo;
	uint32_t                    _primaryFbId;
	OSDBuffer                   _osdBuffers[NUM_OSD_FB];
	VideoBuffer                 *_videoBuffers[NUM_VIDEO_FB];
	int                         _lastOsdX;
	int                         _lastOsdY;
	int                         _lastOsdW;
	int                         _lastOsdH;
	int                         _lastOsd;
	int                         _osdChanged;

	int                         _currentOSDBuffer;
	int                         _currentVideoBuffer;

	struct SwsContext           *_scaleCtx;

	bool paused;

	struct vo_vsync_info vsync_info;
};

struct priv *privHandle;

static int getDisplayVideoBuffer(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height);
static int releaseDisplayVideoBuffer(DisplayVideoBuffer *handle);

static void get_vsync(struct vo *vo, struct vo_vsync_info *info) {
	struct priv *p = vo->priv;
	*info = p->vsync_info;
}

static void wait_events(struct vo *vo, int64_t until_time_us) {
	vo_wait_default(vo, until_time_us);
}

static void wakeup(struct vo *vo) {
}

static int reconfig(struct vo *vo, struct mp_image_params *params) {
	struct priv *p = vo->priv;
	struct mp_rect src, dst;
	struct mp_osd_res osd;

	if (!p->_initialized)
		return -1;

	switch (params->imgfmt) {
	case IMGFMT_NV12:
		p->_dce = 1;
		break;
	case IMGFMT_420P:
		p->_dce = 0;
		break;
	default:
		MP_FATAL(vo, "[omap_drm] config() Error wrong pixel format\n");
		return -1;
	}

	p->_currentOSDBuffer = 0;
	p->_currentVideoBuffer = 0;

	MP_INFO(vo, "Resize: %dx%d\n", vo->dwidth, vo->dheight);

	vo_get_src_dst_rects(vo, &src, &dst, &osd);

	p->vsync_info.vsync_duration = 0;
	p->vsync_info.skipped_vsyncs = -1;
	p->vsync_info.last_queue_display_time = -1;

	vo->want_redraw = true;

	return 0;
}

static VideoBuffer *getVideoBuffer(uint32_t pixelfmt, int width, int height) {
	DisplayVideoBuffer buffer;
	VideoBuffer *videoBuffer;

	if (getDisplayVideoBuffer(&buffer, pixelfmt, width, height) != 0) {
		return NULL;
	}

	videoBuffer = (VideoBuffer *)buffer.priv;
	videoBuffer->db = NULL;

	return videoBuffer;
}

static int getDisplayVideoBuffer(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height) {
	VideoBuffer *videoBuffer;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	uint32_t fbSize;

	if (!privHandle || !privHandle->_initialized || !handle)
		return -1;

	videoBuffer = calloc(1, sizeof(VideoBuffer));

	fbSize = width * height * 3 / 2;
	handle->locked = 0;
	videoBuffer->bo = handle->bo = omap_bo_new(privHandle->_omapDevice, fbSize, OMAP_BO_WC | OMAP_BO_SCANOUT);
	if (!videoBuffer->bo) {
		printf("[omap_drm] getDisplayVideoBuffer() Failed allocate video buffer\n");
		return -1;
	}
	handles[0] = videoBuffer->boHandle = handle->boHandle = omap_bo_handle(handle->bo);
	pitches[0] = width;
	handles[1] = handles[0];
	pitches[1] = pitches[0];
	offsets[1] = width * height;
	if (drmModeAddFB2(privHandle->_fd, width, height,
		            DRM_FORMAT_NV12, handles, pitches, offsets, &videoBuffer->fbId, 0) < 0) {
		printf("[omap_drm] getDisplayVideoBuffer() Failed add video buffer: %s\n", strerror(errno));
		return -1;
	}
	videoBuffer->srcX = 0;
	videoBuffer->srcY = 0;
	videoBuffer->srcWidth = width;
	videoBuffer->srcHeight = height;
	videoBuffer->stride = pitches[0];
	videoBuffer->width = width;
	videoBuffer->height = height;
	videoBuffer->dstX = 0;
	videoBuffer->dstY = 0;
	videoBuffer->dstWidth = width;
	videoBuffer->dstHeight = height;
	videoBuffer->size = omap_bo_size(videoBuffer->bo);
	videoBuffer->ptr = omap_bo_map(videoBuffer->bo);
	if (!videoBuffer->ptr) {
		printf("[omap_drm] getDisplayVideoBuffer() Failed get video frame buffer\n");
		return -1;
	}

	videoBuffer->db = handle;
	handle->priv = videoBuffer;

	return 0;
}

static int releaseVideoBuffer(VideoBuffer *buffer) {
	if (!privHandle || !privHandle->_initialized || !buffer)
		return -1;

	drmModeRmFB(privHandle->_fd, buffer->fbId);

	close(buffer->boHandle);

	omap_bo_del(buffer->bo);

	free(buffer);

	return 0;
}

static int releaseDisplayVideoBuffer(DisplayVideoBuffer *handle) {
	VideoBuffer *videoBuffer;

	if (!privHandle || !privHandle->_initialized || !handle)
		return -1;

	videoBuffer = (VideoBuffer *)handle->priv;
	if (!videoBuffer)
		return -1;

	if (releaseVideoBuffer(videoBuffer) != 0)
		return -1;

	handle->bo = NULL;
	handle->priv = NULL;

	return 0;
}

static void draw_image(struct vo *vo, mp_image_t *mpi) {
	struct priv *p = vo->priv;
	float x, y, w, h;
	int frame_width, frame_height;

	frame_width = mpi->params.w;
	frame_height = mpi->params.h;

	if (p->_dce) {
		DisplayVideoBuffer *db = (DisplayVideoBuffer *)(mpi->priv);
		db->locked = 1;
		p->_videoBuffers[p->_currentVideoBuffer] = (VideoBuffer *)db->priv;
	} else {
		uint8_t *srcPtr[4] = {};
		uint8_t *dstPtr[4] = {};
		int srcStride[4] = {};
		int dstStride[4] = {};
		uint8_t *dst;

		if (!p->_videoBuffers[p->_currentVideoBuffer])
			p->_videoBuffers[p->_currentVideoBuffer] = getVideoBuffer(IMGFMT_NV12, frame_width, frame_height);
		dst = (uint8_t *)p->_videoBuffers[p->_currentVideoBuffer]->ptr;
/*		if (mpi->imgfmt == IMGFMT_420P && (ALIGN2(frame_width, 5) == frame_width)) {
			srcPtr[0] = mpi->planes[0];
			srcPtr[1] = mpi->planes[1];
			srcPtr[2] = mpi->planes[2];
			dstPtr[0] = dst;
			dstPtr[1] = dst + frame_width * frame_height;
			dstPtr[2] = 0;

			yuv420_frame_info.w = frame_width;
			yuv420_frame_info.h = frame_height;
			yuv420_frame_info.dx = 0;
			yuv420_frame_info.dy = 0;
			yuv420_frame_info.dw = frame_width;
			yuv420_frame_info.dh = frame_height;
			yuv420_frame_info.y_stride = mpi->stride[0];
			yuv420_frame_info.uv_stride = mpi->stride[1];

			nv12_frame_info.w = frame_width;
			nv12_frame_info.h = frame_height;
			nv12_frame_info.dx = 0;
			nv12_frame_info.dy = 0;
			nv12_frame_info.dw = frame_width;
			nv12_frame_info.dh = frame_height;
			nv12_frame_info.y_stride = frame_width;
			nv12_frame_info.uv_stride = frame_width;

	printf("[omap_drm] draw_image: %p\n", dst);
			yuv420_to_nv12_open(&yuv420_frame_info, &nv12_frame_info);

			omap_bo_cpu_prep(p->_videoBuffers[p->_currentVideoBuffer]->bo, OMAP_GEM_WRITE);
			yuv420_to_nv12_convert(dstPtr, srcPtr, NULL, NULL);
			omap_bo_cpu_fini(p->_videoBuffers[p->_currentVideoBuffer]->bo, OMAP_GEM_WRITE);
		} else*/ if (mpi->imgfmt == IMGFMT_420P) {
			srcPtr[0] = mpi->planes[0];
			srcPtr[1] = mpi->planes[1];
			srcPtr[2] = mpi->planes[2];
			srcPtr[3] = mpi->planes[3];
			srcStride[0] = mpi->stride[0];
			srcStride[1] = mpi->stride[1];
			srcStride[2] = mpi->stride[2];
			srcStride[3] = mpi->stride[3];
			dstPtr[0] = dst;
			dstPtr[1] = dst + frame_width * frame_height;
			dstPtr[2] = NULL;
			dstPtr[3] = NULL;
			dstStride[0] = frame_width;
			dstStride[1] = frame_width;
			dstStride[2] = 0;
			dstStride[3] = 0;

			if (!p->_scaleCtx) {
				p->_scaleCtx = sws_getContext(frame_width, frame_height, AV_PIX_FMT_YUV420P,
					                   frame_width, frame_height,
					                   AV_PIX_FMT_NV12, SWS_POINT, NULL, NULL, NULL);
				if (!p->_scaleCtx) {
					MP_FATAL(vo, "[omap_drm] Error: draw_image() Can not create scale context!\n");
					goto fail;
				}
			}
			omap_bo_cpu_prep(p->_videoBuffers[p->_currentVideoBuffer]->bo, OMAP_GEM_WRITE);
			sws_scale(p->_scaleCtx, (const uint8_t *const *)srcPtr, srcStride, 0, frame_height, dstPtr, dstStride);
			omap_bo_cpu_fini(p->_videoBuffers[p->_currentVideoBuffer]->bo, OMAP_GEM_WRITE);
		} else {
			MP_FATAL(vo, "[omap_drm] Error: draw_image() Not supported format!\n");
			goto fail;
		}
	}

/*	if ((mpi->flags & 0x800000) || (mpi->w == 720 && (mpi->h == 576 || mpi->h == 480))) { // hack: anisotropic
		x = 0;
		y = 0;
		w = p->_modeInfo.hdisplay;
		h = p->_modeInfo.vdisplay;
	} else */{
		float rw = (float)(mpi->w) / p->_modeInfo.hdisplay;
		float rh = (float)(mpi->h) / p->_modeInfo.vdisplay;
		if (rw >= rh) {
			w = p->_modeInfo.hdisplay;
			h = p->_modeInfo.vdisplay * (rh / rw);
			x = 0;
			y = (p->_modeInfo.vdisplay - h) / 2;
		} else {
			w = p->_modeInfo.hdisplay * (rw / rh);
			h = p->_modeInfo.vdisplay;
			x = (p->_modeInfo.hdisplay - w) / 2;
			y = 0;
		}
	}

	p->_videoBuffers[p->_currentVideoBuffer]->srcX = mpi->x;
	p->_videoBuffers[p->_currentVideoBuffer]->srcY = mpi->y;
	p->_videoBuffers[p->_currentVideoBuffer]->srcWidth = mpi->w;
	p->_videoBuffers[p->_currentVideoBuffer]->srcHeight = mpi->h;

	p->_videoBuffers[p->_currentVideoBuffer]->dstX = x;
	p->_videoBuffers[p->_currentVideoBuffer]->dstY = y;
	p->_videoBuffers[p->_currentVideoBuffer]->dstWidth = w;
	p->_videoBuffers[p->_currentVideoBuffer]->dstHeight = h;

fail:
	return;
}

static void flip_page(struct vo *vo) {
	struct priv *p = vo->priv;
	if (!p->_initialized)
		goto fail;

	if (drmModeSetPlane(p->_fd, p->_videoPlaneId, p->_crtcId,
			p->_videoBuffers[p->_currentVideoBuffer]->fbId, 0,
			p->_videoBuffers[p->_currentVideoBuffer]->dstX,
			p->_videoBuffers[p->_currentVideoBuffer]->dstY,
			p->_videoBuffers[p->_currentVideoBuffer]->dstWidth,
			p->_videoBuffers[p->_currentVideoBuffer]->dstHeight,
			p->_videoBuffers[p->_currentVideoBuffer]->srcX << 16,
			p->_videoBuffers[p->_currentVideoBuffer]->srcY << 16,
			p->_videoBuffers[p->_currentVideoBuffer]->srcWidth << 16,
			p->_videoBuffers[p->_currentVideoBuffer]->srcHeight << 16
			)) {
		MP_FATAL(vo, "[omap_drm] Error: flip() Failed set plane: %s\n", strerror(errno));
		goto fail;
	}
	if (++p->_currentVideoBuffer >= NUM_VIDEO_FB)
		p->_currentVideoBuffer = 0;
	if (p->_videoBuffers[p->_currentVideoBuffer] &&
		p->_videoBuffers[p->_currentVideoBuffer]->db) {
		p->_videoBuffers[p->_currentVideoBuffer]->db->locked = 0;
	}

	if (p->_osdChanged) {
		if (drmModeSetPlane(p->_fd, p->_osdPlaneId, p->_crtcId,
				p->_osdBuffers[p->_currentOSDBuffer].fbId, 0,
				0, 0, p->_modeInfo.hdisplay, p->_modeInfo.vdisplay,
				0, 0, p->_modeInfo.hdisplay << 16, p->_modeInfo.vdisplay << 16
				)) {
			MP_FATAL(vo, "[omap_drm] Error: flip() Failed set plane: %s\n", strerror(errno));
			goto fail;
		}
//		if (++p->_currentOSDBuffer >= NUM_OSD_FB)
//			p->_currentOSDBuffer = 0;
		p->_osdChanged = 0;
	}

fail:

	return;
}

static void uninit(struct vo *vo) {
	struct priv *p = vo->priv;

	for (int i = 0; i < NUM_OSD_FB; i++) {
		if (p->_osdBuffers[i].fbId) {
			drmModeRmFB(p->_fd, p->_osdBuffers[i].fbId);
		}
		if (p->_osdBuffers[i].bo) {
			omap_bo_del(p->_osdBuffers[i].bo);
		}
	}
	memset(p->_osdBuffers, 0, sizeof(OSDBuffer) * NUM_OSD_FB);

	if (!p->_dce) {
		for (int i = 0; i < NUM_VIDEO_FB; i++) {
			if (p->_videoBuffers[i] && p->_videoBuffers[i]->fbId) {
				drmModeRmFB(p->_fd, p->_videoBuffers[i]->fbId);
			}
			if (p->_videoBuffers[i] && p->_videoBuffers[i]->bo) {
				omap_bo_del(p->_videoBuffers[i]->bo);
			}
		}
		memset(p->_videoBuffers, 0, sizeof(VideoBuffer) * NUM_VIDEO_FB);
	}

	if (p->_oldCrtc) {
		drmModeSetCrtc(p->_fd, p->_oldCrtc->crtc_id, p->_oldCrtc->buffer_id,
			       p->_oldCrtc->x, p->_oldCrtc->y, &p->_connectorId, 1, &p->_oldCrtc->mode);
		drmModeFreeCrtc(p->_oldCrtc);
		p->_oldCrtc = NULL;
	}

	if (p->_primaryFbId) {
		drmModeRmFB(p->_fd, p->_primaryFbId);
		p->_primaryFbId = 0;
	}

	if (p->_primaryFbBo) {
		omap_bo_del(p->_primaryFbBo);
		p->_primaryFbBo = NULL;
	}

	if (p->_drmPlaneResources) {
		drmModeFreePlaneResources(p->_drmPlaneResources);
		p->_drmPlaneResources = NULL;
	}

	if (p->_drmResources) {
		drmModeFreeResources(p->_drmResources);
		p->_drmResources = NULL;
	}

	if (p->_omapDevice) {
		omap_device_del(p->_omapDevice);
		p->_omapDevice = NULL;
	}

	if (p->_fd != -1) {
		drmClose(p->_fd);
		p->_fd = -1;
	}

	p->_initialized = 0;
}

static int preinit(struct vo *vo) {
	struct priv *p = vo->priv;
	int modeId = -1, i, j;
	uint32_t handles[4] = { 0 }, pitches[4] = { 0 }, offsets[4] = { 0 };
	drmModeConnectorPtr connector = NULL;
	drmModeObjectPropertiesPtr props;

	privHandle = p;

	p->_fd = drmOpen("omapdrm", NULL);
	if (p->_fd < 0) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed open omapdrm, %s\n", strerror(errno));
		goto fail;
	}

	p->_omapDevice = omap_device_new(p->_fd);
	if (!p->_omapDevice) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed create omap device\n");
		goto fail;
	}

	p->_drmResources = drmModeGetResources(p->_fd);
	if (!p->_drmResources) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed get DRM resources, %s\n", strerror(errno));
		goto fail;
	}

	p->_drmPlaneResources = drmModeGetPlaneResources(p->_fd);
	if (!p->_drmResources) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed get DRM plane resources, %s\n", strerror(errno));
		goto fail;
	}

	p->_connectorId = -1;
	for (i = 0; i < p->_drmResources->count_connectors; i++) {
		connector = drmModeGetConnector(p->_fd, p->_drmResources->connectors[i]);
		if (connector == NULL)
			continue;
		if (connector->connection != DRM_MODE_CONNECTED || connector->count_modes == 0) {
			drmModeFreeConnector(connector);
			continue;
		}
		if (connector->connector_type == DRM_MODE_CONNECTOR_HDMIA ||
		    connector->connector_type == DRM_MODE_CONNECTOR_HDMIB) {
			p->_connectorId = connector->connector_id;
			break;
		}
		drmModeFreeConnector(connector);
	}
	if (p->_connectorId == -1) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find active HDMI connector!\n");
		goto fail;
	}

	for (j = 0; j < connector->count_modes; j++) {
		drmModeModeInfoPtr mode = &connector->modes[j];
		if ((mode->vrefresh >= 60) && (mode->type & DRM_MODE_TYPE_PREFERRED)) {
			modeId = j;
			break;
		}
	}
	if (modeId == -1) {
		uint64_t highestArea = 0;
		for (j = 0; j < connector->count_modes; j++) {
			drmModeModeInfoPtr mode = &connector->modes[j];
			const uint64_t area = mode->hdisplay * mode->vdisplay;
			if ((mode->vrefresh >= 60) && (area > highestArea)) {
				highestArea = area;
				modeId = j;
			}
		}
	}

	p->_crtcId = -1;
	for (i = 0; i < connector->count_encoders; i++) {
		drmModeEncoderPtr encoder = drmModeGetEncoder(p->_fd, connector->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id) {
			p->_crtcId = encoder->crtc_id;
			drmModeFreeEncoder(encoder);
			break;
		}
		drmModeFreeEncoder(encoder);
	}

	if (modeId == -1 || p->_crtcId == -1) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find suitable display output!\n");
		drmModeFreeConnector(connector);
		goto fail;
	}

	p->_modeInfo = connector->modes[modeId];

	drmModeFreeConnector(connector);

	p->_drmPlaneResources = drmModeGetPlaneResources(p->_fd);
	if (!p->_drmPlaneResources) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to plane resources!\n");
		goto fail;
	}
	p->_osdPlaneId = -1;
	for (i = 0; i < p->_drmPlaneResources->count_planes; i++) {
		drmModePlane *plane = drmModeGetPlane(p->_fd, p->_drmPlaneResources->planes[i]);
		if (!plane)
			continue;
		if (plane->crtc_id == 0) {
			p->_osdPlaneId = plane->plane_id;
			drmModeFreePlane(plane);
			break;
		}
		drmModeFreePlane(plane);
	}
	if (p->_osdPlaneId == -1) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find plane!\n");
		goto fail;
	}
	p->_videoPlaneId = -1;
	for (i = 0; i < p->_drmPlaneResources->count_planes; i++) {
		drmModePlane *plane = drmModeGetPlane(p->_fd, p->_drmPlaneResources->planes[i]);
		if (!plane)
			continue;
		if (plane->crtc_id == 0 && plane->plane_id != p->_osdPlaneId) {
			p->_videoPlaneId = plane->plane_id;
			drmModeFreePlane(plane);
			break;
		}
		drmModeFreePlane(plane);
	}

	if (p->_videoPlaneId == -1) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find plane!\n");
		goto fail;
	}

	props = drmModeObjectGetProperties(p->_fd, p->_osdPlaneId, DRM_MODE_OBJECT_PLANE);
	if (!props) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find properties for plane!\n");
		goto fail;
	}
	for (i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(p->_fd, props->props[i]);
		if (prop && strcmp(prop->name, "zorder") == 0 && drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
			if (drmModeObjectSetProperty(p->_fd, p->_osdPlaneId, DRM_MODE_OBJECT_PLANE, props->props[i], 1)) {
				MP_FATAL(vo, "[omap_drm] preinit() Failed to set zorder property for plane!\n");
				goto fail;
			}
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	props = drmModeObjectGetProperties(p->_fd, p->_videoPlaneId, DRM_MODE_OBJECT_PLANE);
	if (!props) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed to find properties for plane!\n");
		goto fail;
		return -1;
	}
	for (i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(p->_fd, props->props[i]);
		if (prop && strcmp(prop->name, "zorder") == 0 && drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
			if (drmModeObjectSetProperty(p->_fd, p->_videoPlaneId, DRM_MODE_OBJECT_PLANE, props->props[i], 0)) {
				MP_FATAL(vo, "[omap_drm] preinit() Failed to set zorder property for plane!\n");
				goto fail;
			}
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	MP_INFO(vo, "[omap_drm] Using display HDMI output: %dx%d@%d\n",
		p->_modeInfo.hdisplay, p->_modeInfo.vdisplay, p->_modeInfo.vrefresh);

	p->_primaryFbBo = omap_bo_new(p->_omapDevice, p->_modeInfo.hdisplay * p->_modeInfo.vdisplay * 4,
		                       OMAP_BO_WC | OMAP_BO_SCANOUT);
	if (!p->_primaryFbBo) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed allocate buffer!\n");
		goto fail;
	}
	handles[0] = omap_bo_handle(p->_primaryFbBo);
	pitches[0] = p->_modeInfo.hdisplay * 4;
	if (drmModeAddFB2(p->_fd, p->_modeInfo.hdisplay, p->_modeInfo.vdisplay, DRM_FORMAT_ARGB8888,
			handles, pitches, offsets, &p->_primaryFbId, 0) < 0) {
		MP_FATAL(vo, "[omap_drm] preinit() Failed add primary buffer: %s\n", strerror(errno));
		goto fail;
	}
	omap_bo_cpu_prep(p->_primaryFbBo, OMAP_GEM_WRITE);
	memset(omap_bo_map(p->_primaryFbBo), 0, omap_bo_size(p->_primaryFbBo));
	omap_bo_cpu_fini(p->_primaryFbBo, OMAP_GEM_WRITE);

	p->_oldCrtc = drmModeGetCrtc(p->_fd, p->_crtcId);
	if (drmModeSetCrtc(p->_fd, p->_crtcId, p->_primaryFbId, 0, 0, &p->_connectorId, 1, &p->_modeInfo) < 0) {
		goto fail;
	}

	for (i = 0; i < NUM_OSD_FB; i++) {
		p->_osdBuffers[i].bo = omap_bo_new(p->_omapDevice, p->_modeInfo.hdisplay * p->_modeInfo.vdisplay * 4,
		                                OMAP_BO_WC | OMAP_BO_SCANOUT);
		if (!p->_osdBuffers[i].bo) {
			MP_FATAL(vo, "[omap_drm] preinit() Failed allocate buffer!\n");
			goto fail;
		}
		handles[0] = omap_bo_handle(p->_osdBuffers[i].bo);
		pitches[0] = p->_modeInfo.hdisplay * 4;
		if (drmModeAddFB2(p->_fd, p->_modeInfo.hdisplay, p->_modeInfo.vdisplay,
			            DRM_FORMAT_ARGB8888,
			            handles, pitches, offsets, &p->_osdBuffers[i].fbId, 0) < 0) {
			MP_FATAL(vo, "[omap_drm] preinit() Failed add video buffer: %s\n", strerror(errno));
			goto fail;
		}
		p->_osdBuffers[i].width = p->_modeInfo.hdisplay;
		p->_osdBuffers[i].height = p->_modeInfo.vdisplay;
		p->_osdBuffers[i].stride = pitches[0];
		p->_osdBuffers[i].size = omap_bo_size(p->_osdBuffers[i].bo);
		p->_osdBuffers[i].ptr = omap_bo_map(p->_osdBuffers[i].bo);
		if (!p->_osdBuffers[i].ptr) {
			MP_FATAL(vo, "[omap_drm] preinit() Failed get primary frame buffer!\n");
			goto fail;
		}
		omap_bo_cpu_prep(p->_osdBuffers[i].bo, OMAP_GEM_WRITE);
		memset(p->_osdBuffers[i].ptr, 0, p->_osdBuffers[i].size);
		omap_bo_cpu_fini(p->_osdBuffers[i].bo, OMAP_GEM_WRITE);
	}

	omap_dce_share.handle.handle = p->_fd;
	omap_dce_share.getDisplayVideoBuffer = &getDisplayVideoBuffer;
	omap_dce_share.releaseDisplayVideoBuffer = &releaseDisplayVideoBuffer;

	p->_scaleCtx = NULL;
	p->_dce = 0;
	p->_currentOSDBuffer = 0;
	p->_currentVideoBuffer = 0;
	p->_lastOsd = 0;
	p->_osdChanged = 0;

	p->vsync_info.vsync_duration = 0;
	p->vsync_info.skipped_vsyncs = -1;
	p->vsync_info.last_queue_display_time = -1;

	p->_initialized = 1;

	return 0;

fail:
	uninit(vo);
	return -1;
}

static int query_format(struct vo *vo, int format) {
	if (format == IMGFMT_420P || format == IMGFMT_NV12)
		return 1;

	return 0;
}

static int control(struct vo *vo, uint32_t request, void *arg) {
	struct priv *p = vo->priv;
	switch (request) {
	case VOCTRL_GET_DISPLAY_FPS: {
		double fps = 0; // todo
		if (fps <= 0)
			break;
		*(double*)arg = fps;
		return VO_TRUE;
	}
	case VOCTRL_PAUSE:
		vo->want_redraw = true;
		p->paused = true;
		return VO_TRUE;
	case VOCTRL_RESUME:
		p->paused = false;
		p->vsync_info.last_queue_display_time = -1;
		p->vsync_info.skipped_vsyncs = 0;
		return VO_TRUE;
	}
	return VO_NOTIMPL;
}

#define OPT_BASE_STRUCT struct priv

const struct vo_driver video_out_omapdrm = {
	.name = "omapdrm",
	.description = "OMAP Direct Rendering Manager",
	.preinit = preinit,
	.query_format = query_format,
	.reconfig = reconfig,
	.control = control,
	.draw_image = draw_image,
	.flip_page = flip_page,
	.get_vsync = get_vsync,
	.uninit = uninit,
	.wait_events = wait_events,
	.wakeup = wakeup,
	.priv_size = sizeof(struct priv),
};
