/* GStreamer
 * Copyright (C) <2012> Wim Taymans <wim.taymans@gmail.com>
 * Copyright (C) <2020> Matthew Waters <matthew@centricular.com>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:gstrtphdrext
 * @title: GstRtphdrext
 * @short_description: Helper methods for dealing with RTP header extensions
 * @see_also: #GstRTPBasePayload, #GstRTPBaseDepayload, gstrtpbuffer
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstrtphdrext.h"

#include <stdlib.h>
#include <string.h>

GST_DEBUG_CATEGORY_STATIC (rtphderext_debug);
#define GST_CAT_DEFAULT (rtphderext_debug)

#define MAX_RTP_EXT_ID 256

/**
 * gst_rtp_hdrext_set_ntp_64:
 * @data: the data to write to
 * @size: the size of @data
 * @ntptime: the NTP time
 *
 * Writes the NTP time in @ntptime to the format required for the NTP-64 header
 * extension. @data must hold at least #GST_RTP_HDREXT_NTP_64_SIZE bytes.
 *
 * Returns: %TRUE on success.
 */
gboolean
gst_rtp_hdrext_set_ntp_64 (gpointer data, guint size, guint64 ntptime)
{
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (size >= GST_RTP_HDREXT_NTP_64_SIZE, FALSE);

  GST_WRITE_UINT64_BE (data, ntptime);

  return TRUE;
}

/**
 * gst_rtp_hdrext_get_ntp_64:
 * @data: (array length=size) (element-type guint8): the data to read from
 * @size: the size of @data
 * @ntptime: (out): the result NTP time
 *
 * Reads the NTP time from the @size NTP-64 extension bytes in @data and store the
 * result in @ntptime.
 *
 * Returns: %TRUE on success.
 */
gboolean
gst_rtp_hdrext_get_ntp_64 (gpointer data, guint size, guint64 * ntptime)
{
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (size >= GST_RTP_HDREXT_NTP_64_SIZE, FALSE);

  if (ntptime)
    *ntptime = GST_READ_UINT64_BE (data);

  return TRUE;
}

/**
 * gst_rtp_hdrext_set_ntp_56:
 * @data: the data to write to
 * @size: the size of @data
 * @ntptime: the NTP time
 *
 * Writes the NTP time in @ntptime to the format required for the NTP-56 header
 * extension. @data must hold at least #GST_RTP_HDREXT_NTP_56_SIZE bytes.
 *
 * Returns: %TRUE on success.
 */
gboolean
gst_rtp_hdrext_set_ntp_56 (gpointer data, guint size, guint64 ntptime)
{
  guint8 *d = data;
  gint i;

  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (size >= GST_RTP_HDREXT_NTP_56_SIZE, FALSE);

  for (i = 0; i < 7; i++) {
    d[6 - i] = ntptime & 0xff;
    ntptime >>= 8;
  }
  return TRUE;
}

/**
 * gst_rtp_hdrext_get_ntp_56:
 * @data: (array length=size) (element-type guint8): the data to read from
 * @size: the size of @data
 * @ntptime: (out): the result NTP time
 *
 * Reads the NTP time from the @size NTP-56 extension bytes in @data and store the
 * result in @ntptime.
 *
 * Returns: %TRUE on success.
 */
gboolean
gst_rtp_hdrext_get_ntp_56 (gpointer data, guint size, guint64 * ntptime)
{
  guint8 *d = data;

  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (size >= GST_RTP_HDREXT_NTP_56_SIZE, FALSE);

  if (ntptime) {
    gint i;

    *ntptime = 0;
    for (i = 0; i < 7; i++) {
      *ntptime <<= 8;
      *ntptime |= d[i];
    }
  }
  return TRUE;
}

#define gst_rtp_header_extension_parent_class parent_class
G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GstRTPHeaderExtension,
    gst_rtp_header_extension, GST_TYPE_ELEMENT,
    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "rtphdrext", 0,
        "RTP Header Extensions");
    );

/**
 * gst_rtp_header_extension_class_set_uri:
 * @klass: the #GstRTPHeaderExtensionClass
 * @uri: the RTP Header extension uri for @klass
 *
 * Set the URI for this RTP header extension implementation.
 *
 * Since: 1.20
 */
void
gst_rtp_header_extension_class_set_uri (GstRTPHeaderExtensionClass * klass,
    const gchar * uri)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_static_metadata (element_class,
      GST_RTP_HEADER_EXTENSION_URI_METADATA_KEY, uri);
}

static void
gst_rtp_header_extension_class_init (GstRTPHeaderExtensionClass * klass)
{
}

static void
gst_rtp_header_extension_init (GstRTPHeaderExtension * ext)
{
  ext->ext_id = G_MAXUINT32;
}

/**
 * gst_rtp_header_extension_get_uri:
 * @ext: a #GstRTPHeaderExtension
 *
 * Returns: the RTP extension URI for this object
 *
 * Since: 1.20
 */
const gchar *
gst_rtp_header_extension_get_uri (GstRTPHeaderExtension * ext)
{
  GstRTPHeaderExtensionClass *klass;
  GstElementClass *element_class;

  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), NULL);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  element_class = GST_ELEMENT_CLASS (klass);

  return gst_element_class_get_metadata (element_class,
      GST_RTP_HEADER_EXTENSION_URI_METADATA_KEY);
}

/**
 * gst_rtp_header_extension_get_supported_flags:
 * @ext: a #GstRTPHeaderExtension
 *
 * Returns: the flags supported by this instance of @ext
 *
 * Since: 1.20
 */
GstRTPHeaderExtensionFlags
gst_rtp_header_extension_get_supported_flags (GstRTPHeaderExtension * ext)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), 0);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->get_supported_flags != NULL, 0);

  return klass->get_supported_flags (ext);
}

/**
 * gst_rtp_header_extension_get_max_size:
 * @ext: a #GstRTPHeaderExtension
 * @input_meta: a #GstBuffer
 *
 * This is used to know how much data a certain header extension will need for
 * both allocating the resulting data, and deciding how much payload data can
 * be generated.
 *
 * Implementations should return as accurate a value as is possible using the
 * information given in the input @buffer.
 *
 * Returns: the maximum size of the data written by this extension
 *
 * Since: 1.20
 */
gsize
gst_rtp_header_extension_get_max_size (GstRTPHeaderExtension * ext,
    const GstBuffer * input_meta)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_BUFFER (input_meta), 0);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), 0);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->get_max_size != NULL, 0);

  return klass->get_max_size (ext, input_meta);
}

/**
 * gst_rtp_header_extension_write:
 * @ext: a #GstRTPHeaderExtension
 * @input_meta: the input #GstBuffer to read information from if necessary
 * @write_flags: #GstRTPHeaderExtensionFlags for how the extension should
 *               be written
 * @output: output RTP #GstBuffer
 * @data: location to write the rtp header extension into
 * @size: size of @data
 *
 * Writes the RTP header extension to @data using information available from
 * the @input_meta.  @data will be sized to be at least the value returned
 * from gst_rtp_header_extension_get_max_size().
 *
 * Returns: the size of the data written, < 0 on failure
 *
 * Since: 1.20
 */
gsize
gst_rtp_header_extension_write (GstRTPHeaderExtension * ext,
    const GstBuffer * input_meta, GstRTPHeaderExtensionFlags write_flags,
    GstBuffer * output, guint8 * data, gsize size)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_BUFFER (input_meta), -1);
  g_return_val_if_fail (GST_IS_BUFFER (output), -1);
  g_return_val_if_fail (gst_buffer_is_writable (output), -1);
  g_return_val_if_fail (data != NULL, -1);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), -1);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, -1);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->write != NULL, -1);

  return klass->write (ext, input_meta, write_flags, output, data, size);
}

/**
 * gst_rtp_header_extension_read:
 * @ext: a #GstRTPHeaderExtension
 * @read_flags: #GstRTPHeaderExtensionFlags for how the extension should
 *               be written
 * @data: location to read the rtp header extension from
 * @size: size of @data
 * @buffer: a #GstBuffer to modify if necessary
 *
 * Read the RTP header extension from @data.
 *
 * Returns: whether the extension could be read from @data
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_read (GstRTPHeaderExtension * ext,
    GstRTPHeaderExtensionFlags read_flags, const guint8 * data, gsize size,
    GstBuffer * buffer)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_BUFFER (buffer), FALSE);
  g_return_val_if_fail (gst_buffer_is_writable (buffer), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, FALSE);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->read != NULL, FALSE);

  return klass->read (ext, read_flags, data, size, buffer);
}

/**
 * gst_rtp_header_extension_get_id:
 * @ext: a #GstRTPHeaderExtension
 *
 * Returns: the RTP extension id configured on @ext
 *
 * Since: 1.20
 */
guint
gst_rtp_header_extension_get_id (GstRTPHeaderExtension * ext)
{
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), 0);

  return ext->ext_id;
}

/**
 * gst_rtp_header_extension_set_id:
 * @ext: a #GstRTPHeaderExtension
 * @ext_id: The id of this extension
 *
 * sets the RTP extension id on @ext
 *
 * Since: 1.20
 */
void
gst_rtp_header_extension_set_id (GstRTPHeaderExtension * ext, guint ext_id)
{
  g_return_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext));
  g_return_if_fail (ext_id < MAX_RTP_EXT_ID);

  ext->ext_id = ext_id;
}

/**
 * gst_rtp_header_extension_set_attributes_from_caps:
 * @ext: a #GstRTPHeaderExtension
 * @caps: the #GstCaps to configure this extension with
 *
 * gst_rtp_header_extension_set_id() must have been called with a valid
 * extension id that is contained in these caps.
 *
 * The only current known caps format is based on the SDP standard as produced
 * by gst_sdp_media_attributes_to_caps().
 *
 * Returns: whether the @caps could be successfully set on @ext.
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_set_attributes_from_caps (GstRTPHeaderExtension * ext,
    const GstCaps * caps)
{
  GstRTPHeaderExtensionClass *klass;
  GstStructure *structure;
  gchar *field_name;

  g_return_val_if_fail (GST_IS_CAPS (caps), FALSE);
  g_return_val_if_fail (gst_caps_is_fixed (caps), FALSE);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, FALSE);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->set_attributes_from_caps != NULL, FALSE);

  structure = gst_caps_get_structure (caps, 0);
  g_return_val_if_fail (structure != NULL, FALSE);
  field_name = g_strdup_printf ("extmap-%u", ext->ext_id);
  g_return_val_if_fail (gst_structure_has_field (structure, field_name), FALSE);
  g_free (field_name);

  return klass->set_attributes_from_caps (ext, caps);
}

/**
 * gst_rtp_header_extension_wants_update_non_rtp_src_caps:
 * @ext: a #GstRTPHeaderExtension
 *
 * Call this function after gst_rtp_header_extension_read() to check if
 * the depayloader's src caps need updating with data received in the last RTP
 * packet.
 *
 * Returns: Whether @ext wants to update depayloader's src caps.
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_wants_update_non_rtp_src_caps (GstRTPHeaderExtension *
    ext)
{
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);

  return ext->wants_update_non_rtp_src_caps;
}

/**
 * gst_rtp_header_extension_set_wants_update_non_rtp_src_caps:
 * @ext: a #GstRTPHeaderExtension
 * @state: TRUE if caps update is needed
 *
 * Call this function in a subclass from #GstRTPHeaderExtensionClass::read to
 * tell the depayloader whether the data just parsed from RTP packet require
 * updating its src (non-RTP) caps. If @state is TRUE, #GstRTPBaseDepayload will
 * eventually invoke gst_rtp_header_extension_update_non_rtp_src_caps() to
 * have the caps update applied. Applying the update also flips the internal
 * "wants update" flag back to FALSE.
 *
 * Since: 1.20
 */
void gst_rtp_header_extension_set_wants_update_non_rtp_src_caps
    (GstRTPHeaderExtension * ext, gboolean state)
{
  g_return_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext));

  ext->wants_update_non_rtp_src_caps = state;
}

/**
 * gst_rtp_header_extension_set_non_rtp_sink_caps:
 * @ext: a #GstRTPHeaderExtension
 * @caps: sink #GstCaps
 *
 * Passes RTP payloader's sink (i.e. not payloaded) @caps to the header
 * extension.
 *
 * Returns: Whether @caps could be read successfully
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_set_non_rtp_sink_caps (GstRTPHeaderExtension * ext,
    const GstCaps * caps)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_CAPS (caps), FALSE);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, FALSE);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);

  if (klass->set_non_rtp_sink_caps) {
    return klass->set_non_rtp_sink_caps (ext, caps);
  }

  return TRUE;
}

/**
 * gst_rtp_header_extension_update_non_rtp_src_caps:
 * @ext: a #GstRTPHeaderExtension
 * @caps: src #GstCaps to modify
 *
 * Updates depayloader src caps based on the information received in RTP header.
 * @caps must be writable as this function may modify them.
 *
 * Returns: whether @caps were modified successfully
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_update_non_rtp_src_caps (GstRTPHeaderExtension * ext,
    GstCaps * caps)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_CAPS (caps), FALSE);
  g_return_val_if_fail (gst_caps_is_writable (caps), FALSE);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, FALSE);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);

  ext->wants_update_non_rtp_src_caps = FALSE;

  if (klass->update_non_rtp_src_caps) {
    return klass->update_non_rtp_src_caps (ext, caps);
  }

  return TRUE;
}

/**
 * gst_rtp_header_extension_set_caps_from_attributes:
 * @ext: a #GstRTPHeaderExtension
 * @caps: writable #GstCaps to modify
 *
 * gst_rtp_header_extension_set_id() must have been called with a valid
 * extension id that is contained in these caps.
 *
 * The only current known caps format is based on the SDP standard as produced
 * by gst_sdp_media_attributes_to_caps().
 *
 * Returns: whether the configured attributes on @ext can successfully be set on
 * 	@caps
 *
 * Since: 1.20
 */
gboolean
gst_rtp_header_extension_set_caps_from_attributes (GstRTPHeaderExtension * ext,
    GstCaps * caps)
{
  GstRTPHeaderExtensionClass *klass;

  g_return_val_if_fail (GST_IS_CAPS (caps), FALSE);
  g_return_val_if_fail (gst_caps_is_writable (caps), FALSE);
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), FALSE);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, FALSE);
  klass = GST_RTP_HEADER_EXTENSION_GET_CLASS (ext);
  g_return_val_if_fail (klass->set_caps_from_attributes != NULL, FALSE);

  return klass->set_caps_from_attributes (ext, caps);
}

/**
 * gst_rtp_header_extension_get_sdp_caps_field_name:
 * @ext: the #GstRTPHeaderExtension
 *
 * Returns: (transfer full): the #GstStructure field name used in SDP-like #GstCaps for this @ext configuration
 *
 * Since: 1.20
 */
gchar *
gst_rtp_header_extension_get_sdp_caps_field_name (GstRTPHeaderExtension * ext)
{
  g_return_val_if_fail (GST_IS_RTP_HEADER_EXTENSION (ext), NULL);
  g_return_val_if_fail (ext->ext_id <= MAX_RTP_EXT_ID, NULL);

  return g_strdup_printf ("extmap-%u", ext->ext_id);
}

/**
 * gst_rtp_header_extension_set_attributes_from_caps_simple_sdp:
 * @ext: the #GstRTPHeaderExtension
 * @caps: #GstCaps to read attributes from
 *
 * Helper implementation for GstRTPExtensionClass::set_attributes_from_caps
 * that retrieves the configured extension id and checks that the
 * corresponding field in the sdp caps is configured for this extension uri.
 * Requires that the extension does not have any attributes or direction
 * advertised in the caps.
 *
 * Returns: whether the attributes in the @caps could be set on @ext successfully
 *
 * Since: 1.20
 */
gboolean
    gst_rtp_header_extension_set_attributes_from_caps_simple_sdp
    (GstRTPHeaderExtension * ext, const GstCaps * caps) {
  gchar *field_name = gst_rtp_header_extension_get_sdp_caps_field_name (ext);
  GstStructure *s = gst_caps_get_structure (caps, 0);
  const gchar *ext_uri;

  if (!(ext_uri = gst_structure_get_string (s, field_name)))
    goto error;

  if (g_strcmp0 (ext_uri, gst_rtp_header_extension_get_uri (ext)) != 0)
    goto error;

  g_free (field_name);
  return TRUE;

error:
  g_free (field_name);
  return FALSE;
}

/**
 * gst_rtp_header_extension_set_caps_from_attributes_simple_sdp:
 * @ext: the #GstRTPHeaderExtension
 * @caps: #GstCaps to write fields into
 *
 * Helper implementation for GstRTPExtensionClass::set_caps_from_attributes
 * that sets the @ext uri on caps with the specified extension id as required
 * for sdp #GstCaps.
 *
 * Requires that the extension does not have any attributes or direction
 * advertised in @caps.
 *
 * Returns: whether the @ext attributes could be set on @caps.
 *
 * Since: 1.20
 */
gboolean
    gst_rtp_header_extension_set_caps_from_attributes_simple_sdp
    (GstRTPHeaderExtension * ext, GstCaps * caps) {
  gchar *field_name = gst_rtp_header_extension_get_sdp_caps_field_name (ext);
  GstStructure *s = gst_caps_get_structure (caps, 0);

  gst_structure_set (s, field_name, G_TYPE_STRING,
      gst_rtp_header_extension_get_uri (ext), NULL);

  g_free (field_name);
  return TRUE;
}

static gboolean
gst_rtp_ext_list_filter (GstPluginFeature * feature, gpointer user_data)
{
  GstElementFactory *factory;
  gchar *uri = user_data;
  const gchar *klass, *factory_uri;
  guint rank;

  /* we only care about element factories */
  if (!GST_IS_ELEMENT_FACTORY (feature))
    return FALSE;

  factory = GST_ELEMENT_FACTORY (feature);

  /* only select elements with autoplugging rank */
  rank = gst_plugin_feature_get_rank (feature);
  if (rank < GST_RANK_MARGINAL)
    return FALSE;

  klass =
      gst_element_factory_get_metadata (factory, GST_ELEMENT_METADATA_KLASS);
  if (!strstr (klass, "Network") || !strstr (klass, "Extension") ||
      !strstr (klass, "RTPHeader"))
    return FALSE;

  factory_uri =
      gst_element_factory_get_metadata (factory,
      GST_RTP_HEADER_EXTENSION_URI_METADATA_KEY);
  if (!factory_uri)
    return FALSE;

  if (uri && g_strcmp0 (uri, factory_uri) != 0)
    return FALSE;

  return TRUE;
}

/**
 * gst_rtp_get_header_extension_list:
 *
 * Retrieve all the currently registered RTP header extensions
 *
 * Returns: (transfer full) (element-type GstRTPHeaderExtension): a #GList of
 *     #GstRTPHeaderExtension. Use gst_plugin_feature_list_free() after use
 *
 * Since: 1.20
 */
GList *
gst_rtp_get_header_extension_list (void)
{
  return gst_registry_feature_filter (gst_registry_get (),
      (GstPluginFeatureFilter) gst_rtp_ext_list_filter, FALSE, NULL);
}

/**
 * gst_rtp_header_extension_create_from_uri:
 * @uri: the rtp header extension URI to search for
 *
 * Returns: (transfer full) (nullable): the #GstRTPHeaderExtension for @uri or %NULL
 *
 * Since: 1.20
 */
GstRTPHeaderExtension *
gst_rtp_header_extension_create_from_uri (const gchar * uri)
{
  GList *l;

  l = gst_registry_feature_filter (gst_registry_get (),
      (GstPluginFeatureFilter) gst_rtp_ext_list_filter, TRUE, (gpointer) uri);
  if (l) {
    GstElementFactory *factory = GST_ELEMENT_FACTORY (l->data);
    GstElement *element = gst_element_factory_create (factory, NULL);

    g_list_free_full (l, (GDestroyNotify) gst_object_unref);

    return GST_RTP_HEADER_EXTENSION (element);
  }

  return NULL;
}
