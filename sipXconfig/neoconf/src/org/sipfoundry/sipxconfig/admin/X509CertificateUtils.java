/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.InputStream;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.io.LineIterator;

public final class X509CertificateUtils {

    private static final String X509_TYPE = "X.509";
    private static final String BEGIN_LINE = "-----BEGIN CERTIFICATE-----";
    private static final String NEW_LINE = System.getProperty("line.separator");

    private X509CertificateUtils() {
        // Utility class - do not instantiate
    }

    /**
     * Creates a X509 certificate based on the provided file path. Strips any preamble information
     * from the original certificate (if any)
     *
     * @param path
     * @return X509 Certificate or null if cannot create
     */
    public static X509Certificate getX509Certificate(String path) {
        X509Certificate cert = null;
        LineIterator it = null;
        InputStream is = null;
        try {
            File sslCertFile = new File(path);
            it = FileUtils.lineIterator(sslCertFile);

            StringBuffer rep = new StringBuffer();
            boolean shouldWrite = false;
            while (it.hasNext()) {
                String line = it.nextLine();
                if (line.equals(BEGIN_LINE)) {
                    shouldWrite = true;
                }
                if (shouldWrite) {
                    rep.append(line);
                    rep.append(NEW_LINE);
                }
            }

            is = new ByteArrayInputStream(rep.toString().getBytes());
            CertificateFactory cf = CertificateFactory.getInstance(X509_TYPE);
            cert = (X509Certificate) cf.generateCertificate(is);
        } catch (Exception ex) {
            cert = null;
        } finally {
            LineIterator.closeQuietly(it);
            IOUtils.closeQuietly(is);
        }
        return cert;
    }

}
