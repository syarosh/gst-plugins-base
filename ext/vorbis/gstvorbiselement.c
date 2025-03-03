/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2020 Huawei Technologies Co., Ltd.
 *   @Author: Stéphane Cerveau <scerveau@collabora.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstvorbiselements.h"
#include "gst/tag/tag.h"

GST_DEBUG_CATEGORY (vorbisenc_debug);
GST_DEBUG_CATEGORY (vorbisdec_debug);
GST_DEBUG_CATEGORY (vorbisparse_debug);
GST_DEBUG_CATEGORY (vorbistag_debug);

void
vorbis_element_init (GstPlugin * plugin)
{
  static gsize res = FALSE;

  if (g_once_init_enter (&res)) {
    gst_tag_register_musicbrainz_tags ();

    GST_DEBUG_CATEGORY_INIT (vorbisenc_debug, "vorbisenc", 0,
        "vorbis encoding element");
    GST_DEBUG_CATEGORY_INIT (vorbisdec_debug, "vorbisdec", 0,
        "vorbis decoding element");
    GST_DEBUG_CATEGORY_INIT (vorbisparse_debug, "vorbisparse", 0,
        "vorbis parsing element");
    GST_DEBUG_CATEGORY_INIT (vorbistag_debug, "vorbistag", 0,
        "vorbis tagging element");

    g_once_init_leave (&res, TRUE);
  }
}
