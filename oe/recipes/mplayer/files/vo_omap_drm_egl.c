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
#include <sys/mman.h>

#include "config.h"
#include "aspect.h"
#include "video_out.h"
#include "video_out_internal.h"
#include "sub/sub.h"
#include "sub/eosd.h"
#include "../mp_core.h"
#include "vo_omap_drm_egl.h"
#include "libavcodec/avcodec.h"

#include <libdrm/omap_drmif.h>
#include <libswscale/swscale.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <gbm/gbm.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

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

static const vo_info_t info = {
	"omap drm egl video driver",
	"omap_drm_egl",
	"",
	""
};

typedef struct {
	int             fd;
	struct gbm_bo   *gbmBo;
	uint32_t        fbId;
} DrmFb;

typedef struct {
	struct          omap_bo *bo;
	int             dmabuf;
	EGLImageKHR     image;
	GLuint          glTexture;
} RenderTexture;

typedef struct {
	int     handle;
} DisplayHandle;

typedef struct {
	void    *priv;
	struct  omap_bo *bo;
} DisplayVideoBuffer;

#define ALIGN2(value, align) (((value) + ((1 << (align)) - 1)) & ~((1 << (align)) - 1))

static int                         _dce;
static int                         _initialized;
static int                         _fd;
static struct omap_device          *_omapDevice;
static struct gbm_device           *_gbmDevice;
static struct gbm_surface          *_gbmSurface;

static drmModeResPtr               _drmResources;
static drmModePlaneResPtr          _drmPlaneResources;
static drmModeCrtcPtr              _oldCrtc;
static drmModeModeInfo             _modeInfo;
static uint32_t                    _connectorId;
static uint32_t                    _crtcId;
static int                         _planeId;

static EGLDisplay                  _eglDisplay;
static EGLSurface                  _eglSurface;
static EGLConfig                   _eglConfig;
static EGLContext                  _eglContext;
static PFNEGLCREATEIMAGEKHRPROC    eglCreateImageKHR;
static PFNEGLDESTROYIMAGEKHRPROC   eglDestroyImageKHR;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
static GLuint                      _vertexShader;
static GLuint                      _fragmentShader;
static GLuint                      _glProgram;
static RenderTexture               *_renderTexture;
static uint32_t                    _fbWidth, _fbHeight;
static uint32_t                    _pixelfmt;
static struct SwsContext           *_scaleCtx;
static int                         _anistropicDVD;
static int                         _interlaced;
static int                         _flipDone;

LIBVO_EXTERN(omap_drm_egl)

static int releaseVideoBuffer(DisplayVideoBuffer *handle);
static int releaseRenderTexture(RenderTexture *texture);
static DrmFb *getDrmFb(struct gbm_bo *gbmBo);

#define EGL_STR_ERROR(value) case value: return #value;
static const char* eglGetErrorStr(EGLint error) {
	switch (error) {
		EGL_STR_ERROR(EGL_SUCCESS)
		EGL_STR_ERROR(EGL_NOT_INITIALIZED)
		EGL_STR_ERROR(EGL_BAD_ACCESS)
		EGL_STR_ERROR(EGL_BAD_ALLOC)
		EGL_STR_ERROR(EGL_BAD_ATTRIBUTE)
		EGL_STR_ERROR(EGL_BAD_CONFIG)
		EGL_STR_ERROR(EGL_BAD_CONTEXT)
		EGL_STR_ERROR(EGL_BAD_CURRENT_SURFACE)
		EGL_STR_ERROR(EGL_BAD_DISPLAY)
		EGL_STR_ERROR(EGL_BAD_MATCH)
		EGL_STR_ERROR(EGL_BAD_NATIVE_PIXMAP)
		EGL_STR_ERROR(EGL_BAD_NATIVE_WINDOW)
		EGL_STR_ERROR(EGL_BAD_PARAMETER)
		EGL_STR_ERROR(EGL_BAD_SURFACE)
		EGL_STR_ERROR(EGL_CONTEXT_LOST)
		default: return "Unknown";
	}
}
#undef EGL_STR_ERROR

static int preinit(const char *arg) {
	int modeId = -1, i, j;
	struct gbm_bo *gbmBo;
	DrmFb *drmFb;
	const char *extensions;
	drmModeConnectorPtr connector = NULL;
	EGLint major, minor;
	EGLint numConfig;
	GLint shaderStatus;
	drmModeObjectPropertiesPtr props;

	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	const EGLint contextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	static const GLchar *vertexShaderSource =
		"attribute vec2 position;                      \n"
		"attribute vec2 texCoord;                      \n"
		"varying   vec2 textureCoords;                 \n"
		"void main()                                   \n"
		"{                                             \n"
		"    textureCoords = texCoord;                 \n"
		"    gl_Position = vec4(position, 0.0, 1.0);   \n"
		"}                                             \n";

	static const GLchar *fragmentShaderSource =
		"#extension GL_OES_EGL_image_external : require               \n"
		"precision mediump float;                                     \n"
		"varying vec2               textureCoords;                    \n"
		"uniform samplerExternalOES textureSampler;                   \n"
		"void main()                                                  \n"
		"{                                                            \n"
		"    gl_FragColor = texture2D(textureSampler, textureCoords); \n"
		"}                                                            \n";

	_fd = drmOpen("omapdrm", NULL);
	if (_fd < 0) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed open omapdrm, %s\n", strerror(errno));
		goto fail;
	}

	_omapDevice = omap_device_new(_fd);
	if (!_omapDevice) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed create omap device\n");
		goto fail;
	}

	_drmResources = drmModeGetResources(_fd);
	if (!_drmResources) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed get DRM resources, %s\n", strerror(errno));
		goto fail;
	}

	_drmPlaneResources = drmModeGetPlaneResources(_fd);
	if (!_drmResources) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed get DRM plane resources, %s\n", strerror(errno));
		goto fail;
	}

	for (int i = 0; i < _drmResources->count_connectors; i++) {
		connector = drmModeGetConnector(_fd, _drmResources->connectors[i]);
		if (connector == NULL)
			continue;
		if (connector->connection != DRM_MODE_CONNECTED || connector->count_modes == 0) {
			drmModeFreeConnector(connector);
			continue;
		}
		if (connector->connector_type == DRM_MODE_CONNECTOR_HDMIA || connector->connector_type == DRM_MODE_CONNECTOR_HDMIB) {
			_connectorId = connector->connector_id;
			drmModeFreeConnector(connector);
			break;
		}
	}

	if (_connectorId == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to find active HDMI connector!\n");
		goto fail;
	}

	_gbmDevice = gbm_create_device(_fd);
	if (!_gbmDevice) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to create gbm device!\n");
		goto fail;
	}

	for (i = 0; i < _drmResources->count_connectors; i++) {
		connector = drmModeGetConnector(_fd, _drmResources->connectors[i]);
		if (connector == NULL)
			continue;
		if (connector->connector_id == _connectorId)
			break;
		drmModeFreeConnector(connector);
	}
	if (!connector) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to find connector!\n");
		return -1;
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
		modeId = 0;
		for (j = 0; j < connector->count_modes; j++) {
			drmModeModeInfoPtr mode = &connector->modes[j];
			const uint64_t area = mode->hdisplay * mode->vdisplay;
			if ((mode->vrefresh >= 60) && (area > highestArea)) {
				highestArea = area;
				modeId = j;
			}
		}
	}

	_modeInfo = connector->modes[modeId];

	_crtcId = -1;
	for (i = 0; i < connector->count_encoders; i++) {
		drmModeEncoderPtr encoder = drmModeGetEncoder(_fd, connector->encoders[i]);
		if (encoder->encoder_id == connector->encoder_id) {
			_crtcId = encoder->crtc_id;
			drmModeFreeEncoder(encoder);
			break;
		}
		drmModeFreeEncoder(encoder);
	}
	drmModeFreeConnector(connector);

	if (modeId == -1 || _crtcId == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to find suitable display output!\n");
		return -1;
	}

	_planeId = -1;
	_drmPlaneResources = drmModeGetPlaneResources(_fd);
	for (i = 0; i < _drmPlaneResources->count_planes; i++) {
		drmModePlane *plane = drmModeGetPlane(_fd, _drmPlaneResources->planes[i]);
		if (!plane)
			continue;
		if (plane->crtc_id == 0) {
			_planeId = plane->plane_id;
			drmModeFreePlane(plane);
			break;
		}
		drmModeFreePlane(plane);
	}
	if (_planeId == -1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to find plane!\n");
		return -1;
	}

	props = drmModeObjectGetProperties(_fd, _planeId, DRM_MODE_OBJECT_PLANE);
	if (!props) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to find properties for plane!\n");
		return -1;
	}
	for (i = 0; i < props->count_props; i++) {
		drmModePropertyPtr prop = drmModeGetProperty(_fd, props->props[i]);
		if (prop && strcmp(prop->name, "zorder") == 0 && drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
			uint64_t value = props->prop_values[i];
			if (drmModeObjectSetProperty(_fd, _planeId, DRM_MODE_OBJECT_PLANE, props->props[i], 0)) {
				mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to set zorder property for plane!\n");
				return -1;
			}
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	_oldCrtc = drmModeGetCrtc(_fd, _crtcId);

	_fbWidth = _modeInfo.hdisplay;
	_fbHeight = _modeInfo.vdisplay;

	mp_msg(MSGT_VO, MSGL_INFO, "[omap_drm_egl] Using display HDMI output: %dx%d@%d\n", _fbWidth, _fbHeight, _modeInfo.vrefresh);

	_gbmSurface = gbm_surface_create(
	    _gbmDevice,
	    _fbWidth,
	    _fbHeight,
	    GBM_FORMAT_XRGB8888,
	    GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
	    );
	if (!_gbmSurface) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to create gbm surface!\n");
		goto fail;
	}

	_eglDisplay = eglGetDisplay((EGLNativeDisplayType)_gbmDevice);
	if (_eglDisplay == EGL_NO_DISPLAY) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to create display!\n");
		goto fail;
	}

	if (!eglInitialize(_eglDisplay, &major, &minor)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to initialize egl, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	mp_msg(MSGT_VO, MSGL_INFO, "[omap_drm_egl] EGL vendor version: \"%s\"\n", eglQueryString(_eglDisplay, EGL_VERSION));

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to bind EGL_OPENGL_ES_API, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	if (!eglChooseConfig(_eglDisplay, configAttribs, &_eglConfig, 1, &numConfig)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to choose config, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}
	if (numConfig != 1) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() More than 1 config: %d\n", numConfig);
		goto fail;
	}

	_eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, contextAttribs);
	if (_eglContext == EGL_NO_CONTEXT) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to create context, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	_eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig, _gbmSurface, NULL);
	if (_eglSurface == EGL_NO_SURFACE) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to create egl surface, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	if (!eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed attach rendering context to egl surface, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	if (!(eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR"))) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() No eglCreateImageKHR!\n");
		goto fail;
	}

	if (!(eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR"))) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() No eglDestroyImageKHR!\n");
		goto fail;
	}

	if (!(glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES"))) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() No glEGLImageTargetTexture2DOES!\n");
		goto fail;
	}

	extensions = (char *)glGetString(GL_EXTENSIONS);
	if (!strstr(extensions, "GL_TI_image_external_raw_video")) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() No GL_TI_image_external_raw_video extension!\n");
		goto fail;
	}

	_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(_vertexShader);
	glGetShaderiv(_vertexShader, GL_COMPILE_STATUS, &shaderStatus);
	if (!shaderStatus) {
		char logStr[shaderStatus];
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Vertex shader compilation failed!\n");
		glGetShaderiv(_vertexShader, GL_INFO_LOG_LENGTH, &shaderStatus);
		if (shaderStatus > 1) {
			glGetShaderInfoLog(_vertexShader, shaderStatus, NULL, logStr);
			mp_msg(MSGT_VO, MSGL_FATAL, logStr);
		}
		goto fail;
	}

	_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(_fragmentShader);
	glGetShaderiv(_fragmentShader, GL_COMPILE_STATUS, &shaderStatus);
	if (!shaderStatus) {
		char logStr[shaderStatus];
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Fragment shader compilation failed!\n");
		glGetShaderiv(_fragmentShader, GL_INFO_LOG_LENGTH, &shaderStatus);
		if (shaderStatus > 1) {
			glGetShaderInfoLog(_fragmentShader, shaderStatus, NULL, logStr);
			mp_msg(MSGT_VO, MSGL_FATAL, logStr);
		}
		goto fail;
	}

	_glProgram = glCreateProgram();

	glAttachShader(_glProgram, _vertexShader);
	glAttachShader(_glProgram, _fragmentShader);

	glBindAttribLocation(_glProgram, 0, "position");
	glBindAttribLocation(_glProgram, 1, "texCoord");

	glLinkProgram(_glProgram);
	glGetProgramiv(_glProgram, GL_LINK_STATUS, &shaderStatus);
	if (!shaderStatus) {
		char logStr[shaderStatus];
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Program linking failed!\n");
		glGetProgramiv(_glProgram, GL_INFO_LOG_LENGTH, &shaderStatus);
		if (shaderStatus > 1) {
			glGetProgramInfoLog(_glProgram, shaderStatus, NULL, logStr);
			mp_msg(MSGT_VO, MSGL_FATAL, logStr);
		}
		goto fail;
	}

	glUseProgram(_glProgram);

	glViewport(0, 0, _fbWidth, _fbHeight);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	if (!eglSwapBuffers(_eglDisplay, _eglSurface)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed to swap buffers, error: %s!\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	gbmBo = gbm_surface_lock_front_buffer(_gbmSurface);
	drmFb = getDrmFb(gbmBo);
	if (!drmFb) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] preinit() Failed get DRM fb\n");
		goto fail;
	}
	if (drmModeSetCrtc(_fd, _crtcId, drmFb->fbId, 0, 0, &_connectorId, 1, &_modeInfo) < 0) {
		gbm_surface_release_buffer(_gbmSurface, gbmBo);
		goto fail;
	}
	gbm_surface_release_buffer(_gbmSurface, gbmBo);

	_scaleCtx = NULL;
	_dce = 0;

	_initialized = 1;

	return 0;

fail:

	if (_vertexShader) {
		glDeleteShader(_vertexShader);
		_vertexShader = 0;
	}
	if (_fragmentShader) {
		glDeleteShader(_fragmentShader);
		_fragmentShader = 0;
	}
	if (_glProgram) {
		glDeleteProgram(_glProgram);
		_glProgram = 0;
	}
	if (_eglSurface) {
		eglDestroySurface(_eglDisplay, _eglSurface);
		_eglSurface = NULL;
	}
	if (_eglContext) {
		eglDestroyContext(_eglDisplay, _eglContext);
		_eglContext = NULL;
	}
	if (_eglDisplay) {
		eglTerminate(_eglDisplay);
		_eglDisplay = NULL;
	}
	if (_gbmSurface) {
		gbm_surface_destroy(_gbmSurface);
		_gbmSurface = NULL;
	}
	if (_gbmDevice != NULL) {
		gbm_device_destroy(_gbmDevice);
		_gbmDevice = NULL;
	}

	if (_drmPlaneResources != NULL) {
		drmModeFreePlaneResources(_drmPlaneResources);
		_drmPlaneResources = NULL;
	}

	if (_drmResources != NULL) {
		drmModeFreeResources(_drmResources);
		_drmResources = NULL;
	}

	if (_omapDevice != NULL) {
		omap_device_del(_omapDevice);
		_omapDevice = NULL;
	}

	if (_fd != -1) {
		drmClose(_fd);
		_fd = -1;
	}

	return -1;
}

static void uninit(void) {
	if (!_initialized)
		return;

	if (_scaleCtx) {
		sws_freeContext(_scaleCtx);
		_scaleCtx = NULL;
	}

	if (_vertexShader) {
		glDeleteShader(_vertexShader);
		_vertexShader = 0;
	}

	if (_fragmentShader) {
		glDeleteShader(_fragmentShader);
		_fragmentShader = 0;
	}

	if (_glProgram) {
		glDeleteProgram(_glProgram);
		_glProgram = 0;
	}

	if (_renderTexture) {
		releaseRenderTexture(_renderTexture);
		_renderTexture = NULL;
	}

	if (_eglDisplay) {
		glFinish();
		eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(_eglDisplay, _eglContext);
		eglDestroySurface(_eglDisplay, _eglSurface);
		eglTerminate(_eglDisplay);
		_eglDisplay = NULL;
	}

	if (_gbmSurface) {
		gbm_surface_destroy(_gbmSurface);
		_gbmSurface = NULL;
	}

	if (_gbmDevice) {
		gbm_device_destroy(_gbmDevice);
		_gbmDevice = NULL;
	}

	if (_oldCrtc) {
		drmModeSetCrtc(_fd, _oldCrtc->crtc_id, _oldCrtc->buffer_id, _oldCrtc->x, _oldCrtc->y, &_connectorId, 1, &_oldCrtc->mode);
		drmModeFreeCrtc(_oldCrtc);
		_oldCrtc = NULL;
	}

	if (_drmPlaneResources)
		drmModeFreePlaneResources(_drmPlaneResources);

	if (_drmResources)
		drmModeFreeResources(_drmResources);

	if (_omapDevice)
		omap_device_del(_omapDevice);

	if (_fd != -1)
		drmClose(_fd);

	_initialized = 0;
}

static int getHandle(DisplayHandle *handle) {
	if (!_initialized || handle == NULL)
		return -1;

	handle->handle = _fd;

	return 0;
}

static int getVideoBuffer(DisplayVideoBuffer *handle, uint32_t pixelfmt, int width, int height) {
	RenderTexture *renderTexture;
	uint32_t fourcc;
	uint32_t stride;
	uint32_t fbSize;

	if (!_initialized || handle == NULL)
		return -1;

	renderTexture = malloc(sizeof(RenderTexture));
	memset(renderTexture, 0, sizeof (RenderTexture));

	if (pixelfmt == IMGFMT_YV12 || pixelfmt == IMGFMT_NV12) {
		fourcc = FOURCC_TI_NV12;
		stride = width;
		fbSize = width * height * 3 / 2;
	} else {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] getVideoBuffer() Can not handle pixel format!\n");
		return -1;
	}

	handle->bo = renderTexture->bo = omap_bo_new(_omapDevice, fbSize, OMAP_BO_WC);
	renderTexture->dmabuf = omap_bo_dmabuf(renderTexture->bo);
	{
		EGLint attr[] = {
			EGL_GL_VIDEO_FOURCC_TI,      (EGLint)fourcc,
			EGL_GL_VIDEO_WIDTH_TI,       (EGLint)width,
			EGL_GL_VIDEO_HEIGHT_TI,      (EGLint)height,
			EGL_GL_VIDEO_BYTE_SIZE_TI,   (EGLint)omap_bo_size(renderTexture->bo),
			EGL_GL_VIDEO_BYTE_STRIDE_TI, (EGLint)stride,
			EGL_GL_VIDEO_YUV_FLAGS_TI,   EGLIMAGE_FLAGS_YUV_CONFORMANT_RANGE | EGLIMAGE_FLAGS_YUV_BT601,
			EGL_NONE
		};
		renderTexture->image = eglCreateImageKHR(_eglDisplay, EGL_NO_CONTEXT, EGL_RAW_VIDEO_TI_DMABUF, (EGLClientBuffer)renderTexture->dmabuf, attr);
		if (renderTexture->image == EGL_NO_IMAGE_KHR) {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] getVideoBuffer() failed to bind texture, error: %s\n", eglGetErrorStr(eglGetError()));
			goto fail;
		}
	}

	glGenTextures(1, &renderTexture->glTexture);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderTexture->glTexture);
	if (glGetError() != GL_NO_ERROR) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] getVideoBuffer() failed to bind texture, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, renderTexture->image);
	if (glGetError() != GL_NO_ERROR) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] getVideoBuffer() failed update texture, error: %s\n", eglGetErrorStr(eglGetError()));
		goto fail;
	}

	handle->priv = renderTexture;

	return 0;

fail:

	if (renderTexture)
		releaseRenderTexture(renderTexture);

	return -1;
}

static RenderTexture *getRenderTexture(uint32_t pixelfmt, int width, int height) {
	DisplayVideoBuffer buffer;

	if (getVideoBuffer(&buffer, pixelfmt, width, height) != 0) {
		return NULL;
	}
	return (RenderTexture *)buffer.priv;
}

static int releaseRenderTexture(RenderTexture *texture) {
	if (!_initialized || texture == NULL || _eglDisplay == NULL)
		return -1;

	if (texture->image) {
		eglDestroyImageKHR(_eglDisplay, texture->image);
		glDeleteTextures(1, &texture->glTexture);
	}

	if (texture->dmabuf)
		close(texture->dmabuf);

	if (texture->bo)
		omap_bo_del(texture->bo);

	free(texture);

	return 0;
}

static int releaseVideoBuffer(DisplayVideoBuffer *handle) {
	RenderTexture *renderTexture;

	if (!_initialized || handle == NULL)
		return -1;

	renderTexture = (RenderTexture *)handle->priv;
	if (renderTexture == NULL || _eglDisplay == NULL)
		return -1;

	if (releaseRenderTexture(renderTexture) != 0)
		return -1;

	handle->bo = NULL;
	handle->priv = NULL;

	return 0;
}

static void drmFbDestroyCallback(struct gbm_bo *gbmBo, void *data) {
	DrmFb *drmFb = data;

	if (drmFb->fbId)
		drmModeRmFB(drmFb->fd, drmFb->fbId);

	free(drmFb);
}

static DrmFb *getDrmFb(struct gbm_bo *gbmBo) {
	uint32_t handles[4] = {}, pitches[4] = {}, offsets[4] = {};
	DrmFb *drmFb;
	int ret;

	drmFb = gbm_bo_get_user_data(gbmBo);
	if (drmFb)
		return drmFb;

	drmFb = malloc(sizeof(DrmFb));
	drmFb->fd = _fd;
	drmFb->gbmBo = gbmBo;

	pitches[0] = gbm_bo_get_stride(gbmBo);
	handles[0] = gbm_bo_get_handle(gbmBo).u32;
	ret = drmModeAddFB2(
	    drmFb->fd,
	    gbm_bo_get_width(gbmBo),
	    gbm_bo_get_height(gbmBo),
	    gbm_bo_get_format(gbmBo),
	    handles,
	    pitches,
	    offsets,
	    &drmFb->fbId,
	    0
	    );
	if (ret < 0) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] getDrmFb() failed add video buffer: %s\n", strerror(errno));
		free(drmFb);
		return NULL;
	}

	gbm_bo_set_user_data(gbmBo, drmFb, drmFbDestroyCallback);

	return drmFb;
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
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] config() Error wrong pixel format\n");
		return -1;
	}

	_pixelfmt = format;

	return 0;
}

static int query_format(uint32_t format) {
	if (format == IMGFMT_YV12 || format == IMGFMT_NV12)
		return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_EOSD | VFCAP_EOSD_UNSCALED |
		       VFCAP_FLIP | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN | VFCAP_ACCEPT_STRIDE | VOCAP_NOSLICES;

	return 0;
}

static uint32_t get_image(mp_image_t *mpi) {
	if (!_dce) {
		return VO_NOTIMPL;
	}

	// todo
	return VO_FALSE;
}

static uint32_t put_image(mp_image_t *mpi) {
	if (!_dce) {
		return VO_NOTIMPL;
	}

	// todo
	return VO_FALSE;
}

static int draw_frame(uint8_t *src[]) {
	// empty
	return VO_FALSE;
}

static int draw_slice(uint8_t *frame_data[], int frame_stride[], int frame_width, int frame_height, int frame_x, int frame_y) {
	RenderTexture *renderTexture;
	uint32_t fourcc;
	uint32_t stride;
	uint32_t fbSize;
	float x, y;
	float cropLeft, cropRight, cropTop, cropBottom;
	int frame_dx, frame_dy, frame_dw, frame_dh;
	GLfloat coords[] = {
		0.0f,  1.0f,
		1.0f,  1.0f,
		0.0f,  0.0f,
		1.0f,  0.0f,
	};
	GLfloat position[8];

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// todo
	frame_dx = frame_dy = 0;
	frame_dw = frame_width;
	frame_dh = frame_height;

	if (_anistropicDVD) {
		x = 1;
		y = 1;
	} else {
		x = (float)(frame_dw) / _fbWidth;
		y = (float)(frame_dh) / _fbHeight;
		if (x > y) {
			y /= x;
			x = 1;
		} else {
			x /= y;
			y = 1;
		}
	}

	position[0] = -x;
	position[1] = -y;
	position[2] =  x;
	position[3] = -y;
	position[4] = -x;
	position[5] =  y;
	position[6] =  x;
	position[7] =  y;

	cropLeft = (float)(frame_dx) / frame_width;
	cropRight = (float)(frame_dw) / frame_width;
	cropTop = (float)(frame_dy) / frame_height;
	cropBottom = (float)(frame_dh) / frame_height;
	coords[0] = coords[4] = cropLeft;
	coords[2] = coords[6] = cropRight;
	coords[5] = coords[7] = cropTop;
	coords[1] = coords[3] = cropBottom;

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, position);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, coords);
	glEnableVertexAttribArray(1);

	if (_dce) {
		DisplayVideoBuffer *db = (DisplayVideoBuffer *)(frame_data[0]);
		renderTexture = (RenderTexture *)db->priv;
	} else if (!_renderTexture) {
		_renderTexture = renderTexture = getRenderTexture(_pixelfmt, frame_width, frame_height);
		if (!_renderTexture) {
			goto fail;
		}
	} else {
		renderTexture = _renderTexture;
	}

	if (!_dce) {
		uint8_t *srcPtr[4] = {};
		uint8_t *dstPtr[4] = {};
		int srcStride[4] = {};
		int dstStride[4] = {};
		uint8_t *dst;

		dst = omap_bo_map(renderTexture->bo);
		if (_pixelfmt == IMGFMT_YV12 && (ALIGN2(frame_width, 5) == frame_width)) {
			srcPtr[0] = frame_data[0];
			srcPtr[1] = frame_data[1];
			srcPtr[2] = frame_data[2];
			dstPtr[0] = dst;
			dstPtr[1] = dst + frame_width * frame_height;
			dstPtr[2] = 0;

			yuv420_frame_info.w = frame_width;
			yuv420_frame_info.h = frame_height;
			yuv420_frame_info.dx = 0;
			yuv420_frame_info.dy = 0;
			yuv420_frame_info.dw = frame_width;
			yuv420_frame_info.dh = frame_height;
			yuv420_frame_info.y_stride = frame_stride[0];
			yuv420_frame_info.uv_stride = frame_stride[1];

			nv12_frame_info.w = frame_width;
			nv12_frame_info.h = frame_height;
			nv12_frame_info.dx = 0;
			nv12_frame_info.dy = 0;
			nv12_frame_info.dw = frame_width;
			nv12_frame_info.dh = frame_height;
			nv12_frame_info.y_stride = frame_width;
			nv12_frame_info.uv_stride = frame_width;

			yuv420_to_nv12_open(&yuv420_frame_info, &nv12_frame_info);

			omap_bo_cpu_prep(renderTexture->bo, OMAP_GEM_WRITE);
			yuv420_to_nv12_convert(dstPtr, srcPtr, NULL, NULL);
			omap_bo_cpu_fini(renderTexture->bo, OMAP_GEM_WRITE);
		} else if (_pixelfmt == IMGFMT_YV12) {
			srcPtr[0] = frame_data[0];
			srcPtr[1] = frame_data[1];
			srcPtr[2] = frame_data[2];
			srcPtr[3] = frame_data[3];
			srcStride[0] = frame_stride[0];
			srcStride[1] = frame_stride[1];
			srcStride[2] = frame_stride[2];
			srcStride[3] = frame_stride[3];
			dstPtr[0] = dst;
			dstPtr[1] = dst + frame_width * frame_height;
			dstPtr[2] = NULL;
			dstPtr[3] = NULL;
			dstStride[0] = frame_width;
			dstStride[1] = frame_width;
			dstStride[2] = 0;
			dstStride[3] = 0;

			if (!_scaleCtx) {
				_scaleCtx = sws_getContext(frame_width, frame_height, AV_PIX_FMT_YUV420P, frame_width, frame_height,
				                           AV_PIX_FMT_NV12, SWS_POINT, NULL, NULL, NULL);
				if (!_scaleCtx) {
					mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] draw_slice() Can not create scale context!\n");
					goto fail;
				}
			}
			omap_bo_cpu_prep(renderTexture->bo, OMAP_GEM_WRITE);
			sws_scale(_scaleCtx, (const uint8_t *const *)srcPtr, srcStride, 0, frame_height, dstPtr, dstStride);
			omap_bo_cpu_fini(renderTexture->bo, OMAP_GEM_WRITE);
		} else {
			mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] draw_slice() Not supported format!\n");
			goto fail;
		}
	}

	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderTexture->glTexture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return VO_TRUE;

fail:
	return VO_FALSE;
}

static void draw_osd(void) {
	// todo
}

static void pageFlipHandler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data) {
	_flipDone = 1;
}

static void flip_page() {
	struct gbm_bo *gbmBo;
	DrmFb *drmFb;

	eglSwapBuffers(_eglDisplay, _eglSurface);
	gbmBo = gbm_surface_lock_front_buffer(_gbmSurface);
	drmFb = getDrmFb(gbmBo);
	if (drmModePageFlip(_fd, _crtcId, drmFb->fbId, DRM_MODE_PAGE_FLIP_EVENT, NULL)) {
		mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] flip() Can not flip buffer! %s\n", strerror(errno));
		goto fail;
	}

	_flipDone = 0;
	while (!_flipDone) {
		int result;
		fd_set fds = {};
		drmEventContext drmEvent = {};
		drmEventContext drmEventContext = {};
		struct timeval timeout = {
			.tv_sec = 3,
			.tv_usec = 0,
		};

		FD_SET(_fd, &fds);
		drmEventContext.version = DRM_EVENT_CONTEXT_VERSION;
		drmEventContext.page_flip_handler = pageFlipHandler;

		result = select(_fd + 1, &fds, NULL, NULL, &timeout);
		if (result <= 0) {
			if (errno == EAGAIN) {
				continue;
			} else {
				mp_msg(MSGT_VO, MSGL_FATAL, "[omap_drm_egl] flip() Timeout on flip buffer!\n");
				goto fail;
			}
		}
		drmHandleEvent(_fd, &drmEventContext);
	}

	gbm_surface_release_buffer(_gbmSurface, gbmBo);

	return;

fail:

	gbm_surface_release_buffer(_gbmSurface, gbmBo);
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
		return VO_TRUE;
	case VOCTRL_DRAW_EOSD:
		if (!data)
			return VO_FALSE;
		// todo
		return VO_TRUE;
	}

	return VO_NOTIMPL;
}
