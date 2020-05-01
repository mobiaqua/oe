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
#include <libswscale/swscale.h>

#include "common/msg.h"
#include "osdep/timer.h"
#include "sub/osd.h"
#include "video/fmt-conversion.h"
#include "video/mp_image.h"
#include "video/sws_utils.h"
#include "vo.h"

struct priv {
    bool active;
    bool waiting_for_flip;
    bool still;
    bool paused;

    int32_t screen_w;
    int32_t screen_h;

    struct mp_rect src;
    struct mp_rect dst;
    struct mp_osd_res osd;

    struct vo_vsync_info vsync_info;
};

static void get_vsync(struct vo *vo, struct vo_vsync_info *info)
{
    struct priv *p = vo->priv;
    *info = p->vsync_info;
}

static void wait_events(struct vo *vo, int64_t until_time_us)
{
    vo_wait_default(vo, until_time_us);
}

static void wakeup(struct vo *vo)
{
}

static int reconfig(struct vo *vo, struct mp_image_params *params)
{
    struct priv *p = vo->priv;

    vo->dwidth = p->screen_w;
    vo->dheight = p->screen_h;
    vo_get_src_dst_rects(vo, &p->src, &p->dst, &p->osd);


    p->vsync_info.vsync_duration = 0;
    p->vsync_info.skipped_vsyncs = -1;
    p->vsync_info.last_queue_display_time = -1;

    vo->want_redraw = true;
    return 0;
}

static void draw_frame(struct vo *vo, struct vo_frame *frame)
{
    struct priv *p = vo->priv;

}

static void flip_page(struct vo *vo)
{
    struct priv *p = vo->priv;

}

static void uninit(struct vo *vo)
{
    struct priv *p = vo->priv;
}

static int preinit(struct vo *vo)
{
    struct priv *p = vo->priv;


    p->vsync_info.vsync_duration = 0;
    p->vsync_info.skipped_vsyncs = -1;
    p->vsync_info.last_queue_display_time = -1;

    return 0;

err:
    uninit(vo);
    return -1;
}

static int query_format(struct vo *vo, int format)
{
    return 1;
}

static int control(struct vo *vo, uint32_t request, void *arg)
{
    struct priv *p = vo->priv;
    switch (request) {
    case VOCTRL_GET_DISPLAY_FPS: {
        double fps = 0;
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
    .draw_frame = draw_frame,
    .flip_page = flip_page,
    .get_vsync = get_vsync,
    .uninit = uninit,
    .wait_events = wait_events,
    .wakeup = wakeup,
    .priv_size = sizeof(struct priv),
};
