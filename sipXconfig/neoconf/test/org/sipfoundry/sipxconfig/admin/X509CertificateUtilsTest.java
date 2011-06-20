/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.security.cert.X509Certificate;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class X509CertificateUtilsTest extends TestCase {

    public void testGetX509Certificate() {
        File sipxCert =  TestHelper.getResourceAsFile(this.getClass(), "sipx.crt");
        X509Certificate cert = X509CertificateUtils.getX509Certificate(sipxCert.getAbsolutePath());
        assertEquals(
                "EMAILADDRESS=root@localhost.localdomain, CN=ca.localhost.localdomain, OU=sipXecs, O=localdomain, L=AnyTown, ST=AnyState, C=US",
                cert.getIssuerDN().getName());

        File equifaxCert = TestHelper.getResourceAsFile(this.getClass(), "equifax.crt");
        X509Certificate cert1 = X509CertificateUtils.getX509Certificate(equifaxCert.getAbsolutePath());
        assertEquals("OU=Equifax Secure Certificate Authority, O=Equifax, C=US", cert1.getIssuerDN().getName());

    }
}
