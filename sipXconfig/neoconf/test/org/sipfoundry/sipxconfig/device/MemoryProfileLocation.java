/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;

/**
 * Temporary profile location implemented as String: it is used mostly as a stub in testing
 * profile generation.
 */
public class MemoryProfileLocation implements ProfileLocation {

    private final Map<String, ByteArrayOutputStream> m_streams = new HashMap<String, ByteArrayOutputStream>();

    public OutputStream getOutput(String profileName) {
        ByteArrayOutputStream stream = m_streams.get(profileName);
        if (stream == null) {
            stream = new ByteArrayOutputStream();
            m_streams.put(profileName, stream);
        }
        return stream;
    }

    public void removeProfile(String profileName) {
        m_streams.remove(profileName);
    }

    public Reader getReader(String profileName) {
        return new StringReader(toString(profileName));
    }

    public Reader getReader() {
        return new StringReader(toString());
    }

    public String toString(String profileName) {
        ByteArrayOutputStream stream = m_streams.get(profileName);
        return getStreamContent(stream);
    }

    @Override
    public String toString() {
        // concatenate all stream
        Collection<ByteArrayOutputStream> values = m_streams.values();
        StringBuilder all = new StringBuilder();
        for (ByteArrayOutputStream stream : values) {
            all.append(getStreamContent(stream));
        }
        return all.toString();
    }

    public void closeOutput(OutputStream stream) {
        IOUtils.closeQuietly(stream);
    }

    private String getStreamContent(ByteArrayOutputStream stream) {
        if (stream == null) {
            return null;
        }
        try {
            return stream.toString("UTF-8");
        } catch (UnsupportedEncodingException e) {
            // UTF-8 is one of the standard encodings... this should never happen
            throw new RuntimeException(e);
        }
    }
}
