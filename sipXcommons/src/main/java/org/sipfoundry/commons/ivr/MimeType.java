/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.ivr;

import org.restlet.data.MediaType;

public enum MimeType {
    WAV("wav", "audio/wav", MediaType.AUDIO_WAV), MP3("mp3", "audio/mpeg", MediaType.AUDIO_MPEG);

    private final String m_format;
    private final String m_mime;
    private final MediaType m_mediaType;

    MimeType(String format, String mime, MediaType mediaType) {
        m_format = format;
        m_mime = mime;
        m_mediaType = mediaType;
    }

    public static String getMimeByFormat(String format) {
        for (MimeType mime : MimeType.values()) {
            if (mime.getFormat().equals(format)) {
                return mime.getMime();
            }
        }
        return null;
    }

    public static MediaType getMediaTypeByMime(String mime) {
        for (MimeType mimeType : MimeType.values()) {
            if (mimeType.getMime().equals(mime)) {
                return mimeType.getMediaType();
            }
        }
        return null;
    }

    public String getFormat() {
        return m_format;
    }

    public String getMime() {
        return m_mime;
    }

    public MediaType getMediaType() {
        return m_mediaType;
    }
}
