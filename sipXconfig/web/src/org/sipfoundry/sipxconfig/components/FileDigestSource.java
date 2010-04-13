/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.io.IOUtils;
import org.apache.tapestry.PageNotFoundException;

public class FileDigestSource {
    private static final int BUFFER_SIZE = 5000;
    private static final int MAX_DIGEST = 5;

    /**
     * Map keyed on resource path of DIGEST checksum (as a string).
     */

    public synchronized String getDigestForResource(Integer userId, String resourcePath) {
        return computeMD5(userId, resourcePath);
    }

    private String computeMD5(Integer id, String resourcePath) {

        InputStream stream = null;

        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");

            stream = new FileInputStream(resourcePath);

            digest.update(id.toString().getBytes());
            digestStream(digest, stream);

            stream.close();
            stream = null;

            byte[] bytes = digest.digest();
            char[] encoded = Hex.encodeHex(bytes);

            return new String(encoded);
        } catch (FileNotFoundException ex) {
            throw new PageNotFoundException("Resource not available. It may have been moved or deleted.");
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        } finally {
            IOUtils.closeQuietly(stream);
        }
    }

    private void digestStream(MessageDigest digest, InputStream stream) throws IOException {
        byte[] buffer = new byte[BUFFER_SIZE];

        for (int i = 0; i < MAX_DIGEST; i++) {
            int length = stream.read(buffer);

            if (length < 0) {
                return;
            }

            digest.update(buffer, 0, length);
        }
    }
}
