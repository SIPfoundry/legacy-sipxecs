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

import org.apache.commons.io.IOUtils;

/**
 * Temporary profile location implemented as String: it is used mostly as a stub in testing
 * profile generation.
 */
public class MemoryProfileLocation implements ProfileLocation {

    private ByteArrayOutputStream m_stream;

    public OutputStream getOutput(String profileName) {
        m_stream = new ByteArrayOutputStream();
        return m_stream;
    }

    public void removeProfile(String profileName) {
        m_stream = null;
    }

    public Reader getReader() {
        return new StringReader(toString());
    }

    public String toString() {
        if (m_stream == null) {
            return null;
        }
        try {
            return m_stream.toString("UTF-8");
        } catch (UnsupportedEncodingException e) {
            // UTF-8 is one of the standard encodings... this should never happen
            throw new RuntimeException(e);
        }
    }

    public void closeOutput(OutputStream stream) {
        IOUtils.closeQuietly(stream);
    }
}
