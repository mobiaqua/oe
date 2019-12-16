/*
 * video output for OMAP4 V4L2 NV12
 *
 * Copyright (C) 2011 Pawel Kolodziejski
 *
 * Some parts inspired by omapfbplay code written by Mans Rullgard
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

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>

#include <timemmgr/tilermem.h>

#include "config.h"
#include "aspect.h"
#include "video_out.h"
#include "video_out_internal.h"
#include "sub/sub.h"
#include "sub/eosd.h"
#include "../mp_core.h"
#include "../libmpcodecs/vd_omap4_dce.h"
#include "libavcodec/avcodec.h"

static vo_info_t info = {
	"omap4_v4l2 video driver",
	"omap4_v4l2",
	"",
	""
};

LIBVO_EXTERN(omap4_v4l2)

static struct fb_var_screeninfo display_info;
static int v4l2_handle = -1, v4l2_handle2 = -1;
static struct v4l2_format v4l2_vout_format, v4l2_vout_format2;
static struct v4l2_crop v4l2_vout_crop, v4l2_vout_crop2;
static int v4l2_num_buffers, v4l2_num_buffers2;
static int v4l2_offset[3], v4l2_stride[3];
static int v4l2_offset2[3], v4l2_stride2[3];
static struct v4l2_buffer tmp_v4l2_buffer, tmp_v4l2_buffer2;

static struct v4l2_buf *v4l2_buffers = NULL, *v4l2_buffers2 = NULL;

static struct v4l2_format overlay_format = { 0 };
static struct v4l2_format overlay_format2 = { 0 };

static int osd_buffer_width, osd_buffer_height;

static struct frame_info {
	unsigned int w;
	unsigned int h;
	unsigned int dx;
	unsigned int dy;
	unsigned int dw;
	unsigned int dh;
	unsigned int y_stride;
	unsigned int uv_stride;
} yuv420_frame_info, v4l2_frame_info;

extern int yuv420_to_nv12_convert(unsigned char *vdst[3], unsigned char *vsrc[3], unsigned char *, unsigned char *);
extern void yuv420_to_nv12_open(struct frame_info *dst, struct frame_info *src);

static int omap4_v4l2_reset_buffers(void);

static int dce;
static int v4l2_draw_buffer_id;
static int v4l2_draw_buffer_id2;
static int interlaced_applied;
static int codec_id;

static int preinit(const char *arg) {
	int fb_handle;
	struct v4l2_capability v4l2_cap;
	struct v4l2_fmtdesc v4l2_formatdesc;

	fb_handle = open("/dev/fb0", O_RDONLY);
	if (fb_handle == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error open /dev/fb0\n");
		return -1;
	}
	if (ioctl(fb_handle, FBIOGET_VSCREENINFO, &display_info) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error get FB screen info (FBIOGET_VSCREENINFO)\n");
		return -1;
	}
	close(fb_handle);
	osd_buffer_width = display_info.xres / 2;
	osd_buffer_height = display_info.yres / 2;

	v4l2_handle = open("/dev/video1", O_RDWR);
	if (v4l2_handle == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error open /dev/video1\n");
		return -1;
	}

	v4l2_handle2 = open("/dev/video2", O_RDWR);
	if (v4l2_handle2 == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error open /dev/video2\n");
		return -1;
	}

	if (ioctl(v4l2_handle, VIDIOC_QUERYCAP, &v4l2_cap) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error query capabilities (VIDIOC_QUERYCAP)\n");
		return -1;
	}
	v4l2_cap.capabilities &= (V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_OUTPUT);
	if (v4l2_cap.capabilities != (V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_OUTPUT)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error V4L2 missing support for video streaming\n");
		return -1;
	}

	if (ioctl(v4l2_handle2, VIDIOC_QUERYCAP, &v4l2_cap) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error query capabilities (VIDIOC_QUERYCAP)\n");
		return -1;
	}
	v4l2_cap.capabilities &= (V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_OUTPUT);
	if (v4l2_cap.capabilities != (V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_OUTPUT)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error V4L2 missing support for video streaming\n");
		return -1;
	}

	v4l2_formatdesc.index = 0;
	v4l2_formatdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	while (!ioctl(v4l2_handle, VIDIOC_ENUM_FMT, &v4l2_formatdesc)) {
		if (v4l2_formatdesc.pixelformat == V4L2_PIX_FMT_NV12)
			break;
		v4l2_formatdesc.index++;
	}
	if (v4l2_formatdesc.pixelformat != V4L2_PIX_FMT_NV12) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error V4L2 missing NV12 format support\n");
		return -1;
	}

	v4l2_formatdesc.index = 0;
	v4l2_formatdesc.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	while (!ioctl(v4l2_handle2, VIDIOC_ENUM_FMT, &v4l2_formatdesc)) {
		if (v4l2_formatdesc.pixelformat == V4L2_PIX_FMT_RGB32)
			break;
		v4l2_formatdesc.index++;
	}
	if (v4l2_formatdesc.pixelformat != V4L2_PIX_FMT_RGB32) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error V4L2 missing RGB32 format support\n");
		return -1;
	}

	dce = 0;
	v4l2_draw_buffer_id = -1;
	v4l2_draw_buffer_id2 = -1;
	v4l2_num_buffers = 0;
	v4l2_num_buffers2 = 0;

	return 0;
}

static int config(uint32_t width, uint32_t height, uint32_t d_width, uint32_t d_height, uint32_t flags, char *title, uint32_t format) {
	int i, k, stream_on_off;
	struct v4l2_requestbuffers request_buf, request_buf2;
	unsigned char *ptr_mmap;
	codec_id = omap4_dce_priv_t.codec_id; // FIXME: hack
	int frame_width, frame_height;

	stream_on_off = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ioctl(v4l2_handle, VIDIOC_STREAMOFF, &stream_on_off);
	ioctl(v4l2_handle2, VIDIOC_STREAMOFF, &stream_on_off);

	switch (format) {
	case IMGFMT_NV12:
		dce = 1;
		break;
	case IMGFMT_YV12:
		dce = 0;
		break;
	default:
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error wrong pixel format at config()\n");
		return -1;
	}

	frame_width = ALIGN2(width, 4);
	frame_height = ALIGN2(height, 4);

	yuv420_frame_info.w = ALIGN2(width, 5);
	yuv420_frame_info.h = ALIGN2(height, 5);
	yuv420_frame_info.dx = 0;
	yuv420_frame_info.dy = 0;
	yuv420_frame_info.dw = width;
	yuv420_frame_info.dh = height;

	v4l2_frame_info.w = display_info.xres;
	v4l2_frame_info.h = display_info.yres;
	v4l2_frame_info.dx = 0;
	v4l2_frame_info.dy = 0;
	v4l2_frame_info.dw = width;
	v4l2_frame_info.dh = height;

	if (v4l2_frame_info.dw * v4l2_frame_info.h > v4l2_frame_info.dh * v4l2_frame_info.w) {
		v4l2_frame_info.dh = v4l2_frame_info.dh * v4l2_frame_info.w / v4l2_frame_info.dw;
		v4l2_frame_info.dw = v4l2_frame_info.w;
		v4l2_frame_info.dy = (v4l2_frame_info.h - v4l2_frame_info.dh) / 2;
	} else {
		v4l2_frame_info.dw = v4l2_frame_info.dw * v4l2_frame_info.h / v4l2_frame_info.dh;
		v4l2_frame_info.dh = v4l2_frame_info.h;
		v4l2_frame_info.dx = (v4l2_frame_info.w - v4l2_frame_info.dw) / 2;
	}

	v4l2_vout_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	v4l2_vout_format2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (dce) {
		v4l2_num_buffers = 30;
		switch (codec_id) {
		case CODEC_ID_H264:
			v4l2_vout_format.fmt.pix.width = ALIGN2(frame_width + (32 * 2), 7);
			v4l2_vout_format.fmt.pix.height = frame_height + 4 * 24;
			break;
		case CODEC_ID_MPEG4:
			v4l2_vout_format.fmt.pix.width = ALIGN2(frame_width + 32, 7);
			v4l2_vout_format.fmt.pix.height = frame_height + 32;
			break;
		case CODEC_ID_MPEG1VIDEO:
		case CODEC_ID_MPEG2VIDEO:
			v4l2_vout_format.fmt.pix.width = frame_width;
			v4l2_vout_format.fmt.pix.height = frame_height;
			break;
		case CODEC_ID_WMV3:
		case CODEC_ID_VC1:
			v4l2_vout_format.fmt.pix.width = ALIGN2(frame_width + (32 * 2), 7);
			v4l2_vout_format.fmt.pix.height = (ALIGN2(frame_height / 2, 4) * 2) + 2 * 40;
			break;
		default:
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Unsupported codec %08x\n", codec_id);
			return -1;
		}
	} else {
		v4l2_num_buffers = 3;
		v4l2_vout_format.fmt.pix.width = ALIGN2(width, 5);
		v4l2_vout_format.fmt.pix.height = ALIGN2(height, 5);
	}
	v4l2_num_buffers2 = 6;
	v4l2_vout_format2.fmt.pix.width = osd_buffer_width;
	v4l2_vout_format2.fmt.pix.height = osd_buffer_height;

	v4l2_vout_format.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
	v4l2_vout_format.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(v4l2_handle, VIDIOC_S_FMT, &v4l2_vout_format) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error setup video format (VIDIOC_S_FMT)\n");
		return -1;
	}

	v4l2_vout_format2.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
	v4l2_vout_format2.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(v4l2_handle2, VIDIOC_S_FMT, &v4l2_vout_format2) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error setup video format (VIDIOC_S_FMT)\n");
		return -1;
	}

	v4l2_offset[0] = 0;
	v4l2_stride[0] = v4l2_vout_format.fmt.pix.bytesperline;
	v4l2_offset[1] = v4l2_vout_format.fmt.pix.height * v4l2_vout_format.fmt.pix.bytesperline;
	v4l2_stride[1] = v4l2_vout_format.fmt.pix.bytesperline;
	v4l2_offset[2] = 0;
	v4l2_stride[2] = 0;

	v4l2_offset2[0] = 0;
	v4l2_stride2[0] = v4l2_vout_format2.fmt.pix.bytesperline;
	v4l2_offset2[1] = 0;
	v4l2_stride2[1] = 0;
	v4l2_offset2[2] = 0;
	v4l2_stride2[2] = 0;

	v4l2_frame_info.y_stride = v4l2_stride[0];
	v4l2_frame_info.uv_stride = v4l2_stride[1];

	// allocate v4l2 driver buffers
	request_buf.count = v4l2_num_buffers;
	request_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	request_buf.memory = V4L2_MEMORY_MMAP;
	if (ioctl(v4l2_handle, VIDIOC_REQBUFS, &request_buf) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error request buffers(VIDIOC_REQBUFS)\n");
		return -1;
	}
	mp_msg(MSGT_VO, MSGL_INFO, "[omap4_v4l2] Allocated V4L2 bufffers: %d\n", request_buf.count);

	request_buf2.count = v4l2_num_buffers2;
	request_buf2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	request_buf2.memory = V4L2_MEMORY_MMAP;
	if (ioctl(v4l2_handle2, VIDIOC_REQBUFS, &request_buf2) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error request buffers(VIDIOC_REQBUFS)\n");
		return -1;
	}

	// prepare info array of v4l2 buffers
	v4l2_num_buffers = request_buf.count;
	v4l2_buffers = malloc(v4l2_num_buffers * sizeof(struct v4l2_buf));
	if (!v4l2_buffers) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error allocate v4l2 info buffers\n");
		return -1;
	}

	v4l2_num_buffers2 = request_buf2.count;
	v4l2_buffers2 = malloc(v4l2_num_buffers2 * sizeof(struct v4l2_buf));
	if (!v4l2_buffers2) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error allocate v4l2 info buffers\n");
		return -1;
	}

	for (i = 0; i < v4l2_num_buffers; i++) {
		// initialize v4l2 driver buffer
		v4l2_buffers[i].buffer.index = i;
		v4l2_buffers[i].buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		v4l2_buffers[i].buffer.memory = V4L2_MEMORY_MMAP;
		if (ioctl(v4l2_handle, VIDIOC_QUERYBUF, &v4l2_buffers[i].buffer) == -1) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error query buffers (VIDIOC_QUERYBUF)\n");
			goto error;
		}

		// mmap v4l2 driver output buffer
		ptr_mmap = mmap(NULL, v4l2_buffers[i].buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_handle, v4l2_buffers[i].buffer.m.offset);
		if (ptr_mmap == MAP_FAILED) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error mmap video buffer\n");
			goto error;
		}
		// clean planes buffer
		memset(ptr_mmap, 0, v4l2_buffers[i].buffer.length);
		// setup pointers to planes
		for (k = 0; k < 3; k++) {
			v4l2_buffers[i].plane[k] = ptr_mmap + v4l2_offset[k];
		}
		v4l2_buffers[i].plane_p[0] = (unsigned char *)TilerMem_VirtToPhys(v4l2_buffers[i].plane[0]);
		v4l2_buffers[i].plane_p[1] = (unsigned char *)TilerMem_VirtToPhys(v4l2_buffers[i].plane[1]);
		if (!dce) {
			if (ioctl(v4l2_handle, VIDIOC_QBUF, &v4l2_buffers[i].buffer) == -1) {
				mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error queue buffer (VIDIOC_QBUF)\n");
				goto error;
			}
		}
		v4l2_buffers[i].used = false;
		v4l2_buffers[i].to_free = 0;
		v4l2_buffers[i].locked = false;
	}

	if (dce) {
		for (i = 0; i < 3; i++) {
			if (ioctl(v4l2_handle, VIDIOC_QBUF, &v4l2_buffers[i].buffer) == -1) {
				mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error queue buffer (VIDIOC_QBUF)\n");
				goto error;
			}
			v4l2_buffers[i].used = true;
			v4l2_buffers[i].locked = true;
		}
	}

	for (i = 0; i < v4l2_num_buffers2; i++) {
		// initialize v4l2 driver buffer
		v4l2_buffers2[i].buffer.index = i;
		v4l2_buffers2[i].buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		v4l2_buffers2[i].buffer.memory = V4L2_MEMORY_MMAP;
		if (ioctl(v4l2_handle2, VIDIOC_QUERYBUF, &v4l2_buffers2[i].buffer) == -1) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error query buffers (VIDIOC_QUERYBUF)\n");
			goto error;
		}

		// mmap v4l2 driver output buffer
		ptr_mmap = mmap(NULL, v4l2_buffers2[i].buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_handle2, v4l2_buffers2[i].buffer.m.offset);
		if (ptr_mmap == MAP_FAILED) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error mmap video buffer\n");
			goto error;
		}
		// clean planes buffer
		memset(ptr_mmap, 0, v4l2_buffers2[i].buffer.length);
		// setup pointers to planes
		for (k = 0; k < 3; k++) {
			v4l2_buffers2[i].plane[k] = ptr_mmap + v4l2_offset2[k];
		}
		v4l2_buffers2[i].used = false;
	}

	for (i = 0; i < 5; i++) {
		if (ioctl(v4l2_handle2, VIDIOC_QBUF, &v4l2_buffers2[i].buffer) == -1) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error queue buffer (VIDIOC_QBUF)\n");
			goto error;
		}
		v4l2_buffers2[i].used = true;
	}
	v4l2_draw_buffer_id2 = 5;

	v4l2_vout_crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	v4l2_vout_crop.c.left = 0;
	v4l2_vout_crop.c.top = 0;
	v4l2_vout_crop.c.width = width;
	v4l2_vout_crop.c.height = height;
	if (ioctl(v4l2_handle, VIDIOC_S_CROP, &v4l2_vout_crop) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error crop video output (VIDIOC_S_CROP)\n");
		goto error;
	}

	v4l2_vout_crop2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	v4l2_vout_crop2.c.left = 0;
	v4l2_vout_crop2.c.top = 0;
	v4l2_vout_crop2.c.width = osd_buffer_width;
	v4l2_vout_crop2.c.height = osd_buffer_height;
	if (ioctl(v4l2_handle2, VIDIOC_S_CROP, &v4l2_vout_crop2) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error crop video output (VIDIOC_S_CROP)\n");
		goto error;
	}

	overlay_format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	overlay_format.fmt.win.w.left = v4l2_frame_info.dx;
	overlay_format.fmt.win.w.width = v4l2_frame_info.dw;

	// hack for anistropic mpeg2 files: DVD 720x576
	if (codec_id == CODEC_ID_MPEG2VIDEO && width == 720 && height == 576) {
		overlay_format.fmt.win.w.left = 0;
		overlay_format.fmt.win.w.width = v4l2_frame_info.w;
	} else {
		overlay_format.fmt.win.w.left = v4l2_frame_info.dx;
		overlay_format.fmt.win.w.width = v4l2_frame_info.dw;
	}
	overlay_format.fmt.win.w.top = v4l2_frame_info.dy;
	overlay_format.fmt.win.w.height = v4l2_frame_info.dh;
	overlay_format.fmt.win.field = V4L2_FIELD_NONE;
	overlay_format.fmt.win.global_alpha = 255;
	if (ioctl(v4l2_handle, VIDIOC_S_FMT, &overlay_format) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error configure overlay (VIDIOC_S_FMT)\n");
		goto error;
	}

	overlay_format2.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	overlay_format2.fmt.win.w.left = 0;
	overlay_format2.fmt.win.w.width = display_info.xres;
	overlay_format2.fmt.win.w.top = 0;
	overlay_format2.fmt.win.w.height = display_info.yres;
	overlay_format2.fmt.win.field = V4L2_FIELD_NONE;
	overlay_format2.fmt.win.global_alpha = 255;
	if (ioctl(v4l2_handle2, VIDIOC_S_FMT, &overlay_format2) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error configure overlay (VIDIOC_S_FMT)\n");
		goto error;
	}

	stream_on_off = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(v4l2_handle, VIDIOC_STREAMON, &stream_on_off) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error turn on streaming (VIDIOC_STREAMON)\n");
		goto error;
	}

	stream_on_off = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(v4l2_handle2, VIDIOC_STREAMON, &stream_on_off) == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error turn on streaming (VIDIOC_STREAMON)\n");
		goto error;
	}

	tmp_v4l2_buffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	tmp_v4l2_buffer.memory = V4L2_MEMORY_MMAP;
	tmp_v4l2_buffer2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	tmp_v4l2_buffer2.memory = V4L2_MEMORY_MMAP;

	// FIXME: no way for now how to access to sh handle
	omap4_dce_priv_t.reset_buffers = omap4_v4l2_reset_buffers;

	interlaced_applied = false;

	return 0;
error:
	free(v4l2_buffers);
	free(v4l2_buffers2);
	v4l2_buffers = NULL;
	v4l2_buffers2 = NULL;
	return -1;
}

static void draw_osd_stride(int x0, int y0, int w, int h, unsigned char *src, unsigned char *srca, int stride) {
	vo_draw_alpha_rgb32(w, h, src, srca, stride,
			v4l2_buffers2[v4l2_draw_buffer_id2].plane[0] + y0 * v4l2_stride2[0] + x0 * 4, v4l2_stride2[0]);
}

static void draw_osd(void) {
	vo_draw_text(osd_buffer_width, osd_buffer_height, draw_osd_stride);
}

static int draw_frame(uint8_t *src[]) {
	mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: draw_frame() that should be never called\n");
	return VO_FALSE;
}

static int draw_slice(uint8_t *src[], int stride[], int w, int h, int x, int y) {
	if (dce) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: draw_slice() only for software decoding\n");
		return VO_FALSE;
	}

	if ((x != 0) || (y != 0)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error draw_slice() offsets x, y: %d,%d must be 0, 0\n", x, y);
		return VO_FALSE;
	}

	yuv420_frame_info.y_stride = stride[0];
	yuv420_frame_info.uv_stride = stride[1];
	yuv420_to_nv12_open(&yuv420_frame_info, &v4l2_frame_info);

	ioctl(v4l2_handle, VIDIOC_DQBUF, &tmp_v4l2_buffer);

	yuv420_to_nv12_convert(v4l2_buffers[tmp_v4l2_buffer.index].plane, src, NULL, NULL);

	return VO_TRUE;
}

static uint32_t get_image(mp_image_t *mpi) {
	int i, buffer_id;

	if (!dce) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: get_image() only for hardware decoding\n");
		return VO_NOTIMPL;
	}
	if ((mpi->type == MP_IMGTYPE_TEMP) && (mpi->flags & MP_IMGFLAG_ACCEPT_STRIDE)) {
		buffer_id = -1;
		for (i = 0; i < v4l2_num_buffers; i++) {
			if (!v4l2_buffers[i].used) {
				buffer_id = i;
				break;
			}
		}
		if (buffer_id == -1) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: no free buffer\n");
			exit_player(EXIT_QUIT);
		}
		for (i = 0; i < v4l2_num_buffers; i++) {
			if (v4l2_buffers[i].to_free && !v4l2_buffers[i].locked) {
				if (v4l2_buffers[i].to_free++ < 4)
					continue;
				v4l2_buffers[i].used = false;
				v4l2_buffers[i].to_free = 0;
				v4l2_buffers[i].locked = false;
			}
		}
		v4l2_buffers[buffer_id].used = true;
		v4l2_buffers[buffer_id].to_free = 0;
		v4l2_buffers[buffer_id].locked = false;
		mpi->priv = &v4l2_buffers[buffer_id];
		mpi->flags |= MP_IMGFLAG_DIRECT | MP_IMGFLAG_DRAW_CALLBACK;
		return VO_TRUE;
	} else {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: get_image() only for MP_IMGTYPE_TEMP and MP_IMGFLAG_ACCEPT_STRIDE\n");
		return VO_FALSE;
	}
}

static uint32_t put_image(mp_image_t *mpi) {
	if (!dce) {
		return VO_NOTIMPL;
	}
	if ((mpi->type == MP_IMGTYPE_TEMP) && (mpi->flags & MP_IMGFLAG_ACCEPT_STRIDE) && (mpi->flags && MP_IMGFLAG_DIRECT)) {
		int force = false;
		v4l2_draw_buffer_id = ((struct v4l2_buf *)mpi->priv)->buffer.index;
		if (((struct v4l2_buf *)mpi->priv)->interlaced && !interlaced_applied)
			force = true;
		if (force || v4l2_vout_crop.c.left != mpi->x || v4l2_vout_crop.c.top != mpi->y ||
				v4l2_vout_crop.c.width != mpi->width || v4l2_vout_crop.c.height != mpi->height) {
			v4l2_vout_crop.c.left = mpi->x;
			v4l2_vout_crop.c.top = mpi->y;
			v4l2_vout_crop.c.width = mpi->width;
			v4l2_vout_crop.c.height = mpi->height - 1; // FIXME/HACK: sacrifice one line to prevent green flickering
			ioctl(v4l2_handle, VIDIOC_S_CROP, &v4l2_vout_crop);
		}
		if (((struct v4l2_buf *)mpi->priv)->interlaced && !interlaced_applied) {
			overlay_format.fmt.win.w.height = v4l2_frame_info.dh;
			ioctl(v4l2_handle, VIDIOC_S_FMT, &overlay_format);
			interlaced_applied = true;
		}
		if (mpi->flags & 0x800000) { // special case to force flip
			flip_page();
		}
		return VO_TRUE;
	} else {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: put_image() only for MP_IMGTYPE_TEMP and MP_IMGFLAG_ACCEPT_STRIDE | MP_IMGFLAG_DIRECT\n");
		return VO_FALSE;
	}
}

static void flip_page(void) {
	int line;

	if (dce) {
		ioctl(v4l2_handle, VIDIOC_QBUF, &v4l2_buffers[v4l2_draw_buffer_id].buffer);
		v4l2_buffers[v4l2_draw_buffer_id].locked = true;
		ioctl(v4l2_handle, VIDIOC_DQBUF, &tmp_v4l2_buffer);
		v4l2_buffers[tmp_v4l2_buffer.index].locked = false;
	} else {
		ioctl(v4l2_handle, VIDIOC_QBUF, &v4l2_buffers[tmp_v4l2_buffer.index].buffer);
	}
	ioctl(v4l2_handle2, VIDIOC_QBUF, &v4l2_buffers2[v4l2_draw_buffer_id2].buffer);
	ioctl(v4l2_handle2, VIDIOC_DQBUF, &tmp_v4l2_buffer2);
	v4l2_draw_buffer_id2 = tmp_v4l2_buffer2.index;
	// FIXME: temporary cleanup whole buffer
	for (line = 0; line < osd_buffer_height; line++) {
		memset(v4l2_buffers2[v4l2_draw_buffer_id2].plane[0] + line * v4l2_stride2[0], 0, v4l2_stride2[0]);
	}
}

static int omap4_v4l2_reset_buffers(void) {
	int i;

	if (!dce) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap4_v4l2] Error: reset_buffers() only for hardware decoding\n");
		return VO_FALSE;
	}

	for (i = 0; i < v4l2_num_buffers; i++) {
		if (v4l2_buffers[i].used && !v4l2_buffers[i].locked) {
			v4l2_buffers[i].used = false;
			v4l2_buffers[i].to_free = 0;
		}
	}

	return VO_TRUE;
}

static int query_format(uint32_t format) {
	if (format == IMGFMT_YV12 || format == IMGFMT_NV12)
		return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_EOSD | VFCAP_EOSD_UNSCALED |
				VFCAP_SWSCALE | VFCAP_ACCEPT_STRIDE | VOCAP_NOSLICES;

	return 0;
}

static void uninit() {
	int i, stream_off;

	if (vo_config_count > 0) {
		stream_off = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		ioctl(v4l2_handle, VIDIOC_STREAMOFF, &stream_off);
		ioctl(v4l2_handle2, VIDIOC_STREAMOFF, &stream_off);

		if (v4l2_buffers) {
			for (i = 0; i < v4l2_num_buffers; i++) {
				if (v4l2_buffers[i].plane[0]) {
					munmap(v4l2_buffers[i].plane[0], v4l2_buffers[i].buffer.length);
				}
			}
			free(v4l2_buffers);
			v4l2_buffers = NULL;
		}

		if (v4l2_buffers2) {
			for (i = 0; i < v4l2_num_buffers2; i++) {
				if (v4l2_buffers2[i].plane[0]) {
					munmap(v4l2_buffers2[i].plane[0], v4l2_buffers2[i].buffer.length);
				}
			}
			free(v4l2_buffers2);
			v4l2_buffers2 = NULL;
		}
	}
	if (v4l2_handle != -1) {
		close(v4l2_handle);
		v4l2_handle = -1;
	}
	if (v4l2_handle2 != -1) {
		close(v4l2_handle2);
		v4l2_handle2 = -1;
	}
}

static int control(uint32_t request, void *data) {
	switch (request) {
	case VOCTRL_QUERY_FORMAT:
		return query_format(*((uint32_t *)data));
	case VOCTRL_FULLSCREEN:
		return VO_TRUE;
	case VOCTRL_UPDATE_SCREENINFO:
		vo_screenwidth = display_info.xres;
		vo_screenheight = display_info.yres;
		aspect_save_screenres(vo_screenwidth, vo_screenheight);
		return VO_TRUE;
	case VOCTRL_GET_IMAGE:
		return get_image(data);
	case VOCTRL_DRAW_IMAGE:
		return put_image(data);
	case VOCTRL_GET_EOSD_RES: {
			struct mp_eosd_settings *r = data;
			r->mt = r->mb = r->ml = r->mr = 0;
			r->srcw = osd_buffer_width;
			r->srch = osd_buffer_height;
			r->w = osd_buffer_width;
			r->h = osd_buffer_height;
		}
		return VO_TRUE;
	case VOCTRL_DRAW_EOSD:
		if (!data)
			return VO_FALSE;
//		generate_eosd(data);
//		draw_eosd();
		return VO_TRUE;
	}

	return VO_NOTIMPL;
}

static void check_events(void) {}
