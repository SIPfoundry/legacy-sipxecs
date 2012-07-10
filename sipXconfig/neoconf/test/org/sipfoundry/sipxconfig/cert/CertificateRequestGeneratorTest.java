/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import static org.junit.Assert.assertTrue;

import java.io.IOException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.junit.Test;


public class CertificateRequestGeneratorTest {
    
    @Test
    public void generate() throws IOException {
        CertificateRequestGenerator csr = new CertificateRequestGenerator("example.org", "www.example.org");
        String cert = IOUtils.toString(getClass().getResourceAsStream("test.crt"));
        String key = IOUtils.toString(getClass().getResourceAsStream("test.key"));
        String csrTxt = csr.getCertificateRequestText(cert, key);
        assertTrue(StringUtils.isNotBlank(csrTxt));
    }
}
