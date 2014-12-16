/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStream;
import java.io.InputStreamReader;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.cert.CertificateManager;

public class SiteConfigurationTest extends PolycomXmlTestCase {
    private CertificateManager m_certificateManager;

    @Override
    protected void setUp() throws Exception {
        setUp404150TestsMwiMohRlsEnabled();
        String cert = "Version: 3 " + "Serial Number: 1378803401547 " + "Signature Algorithm: SHA1WITHRSA "
                + "Issuer: C=US,ST=AnyState,L=AnyTown,O=ezuce.ro,OU=sipXecs,CN=ca.ezuce.ro,E=root@ezuce.ro "
                + "Not Before: Tue Sep 10 10:56:41 EEST 2013 " + "Not After: Sun Sep 10 10:56:41 EEST 2023 "
                + "Subject: C=US,ST=AnyState,L=AnyTown,O=ezuce.ro,OU=sipXecs,CN=ca.ezuce.ro,E=root@ezuce.ro "
                + "-----BEGIN CERTIFICATE----- "
                + "MIIDrjCCApagAwIBAgIGAUEHFkNLMA0GCSqGSIb3DQEBBQUAMIGLMQswCQYDVQQG "
                + "EwJVUzERMA8GA1UECAwIQW55U3RhdGUxEDAOBgNVBAcMB0FueVRvd24xETAPBgNV "
                + "BAoMCGV6dWNlLnJvMRAwDgYDVQQLDAdzaXBYZWNzMRQwEgYDVQQDDAtjYS5lenVj "
                + "ZS5ybzEcMBoGCSqGSIb3DQEJARYNcm9vdEBlenVjZS5ybzAeFw0xMzA5MTAwNzU2 "
                + "NDFaFw0yMzA5MTAwNzU2NDFaMIGLMQswCQYDVQQGEwJVUzERMA8GA1UECAwIQW55 "
                + "U3RhdGUxEDAOBgNVBAcMB0FueVRvd24xETAPBgNVBAoMCGV6dWNlLnJvMRAwDgYD "
                + "VQQLDAdzaXBYZWNzMRQwEgYDVQQDDAtjYS5lenVjZS5ybzEcMBoGCSqGSIb3DQEJ "
                + "ARYNcm9vdEBlenVjZS5ybzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB "
                + "AKVIZjpxKbmCQ9MZEu/h5AljCmpn2Hjq8D6Q/7ifXa7/9FysnOXQwDKhCm6PBjKJ "
                + "BdLyojkl9k0ZEcScw6xmsgaHncFGe31yYhm8vW1zCQbqLGprf+eMKXnNOfZfyrZ1 "
                + "DUaKUr0xVGEpSvsf6nwYvt6TUZ3vfW9/0M0NaoFTWszuO7ekZG4UUTzjzRV/mymk "
                + "4MjsdxnZSM7aA459mEgMNN8l6VW8smU3f9/mgRAb/hhp/BviOP+bpZzZGreC83/H "
                + "mgmgRbGXULE538DWIwLK0IyctEDAl/j2zDsqem2LcHwqIjWhM4OzmpswXtJbkMJ1 "
                + "/ngaWLOYWmDo/Tspl+RbtTMCAwEAAaMWMBQwEgYDVR0TAQH/BAgwBgEB/wIBADAN "
                + "BgkqhkiG9w0BAQUFAAOCAQEALR0OPImXQCN7iYZ6BQT5RV+Y/PyKOGQPKbinLFpf "
                + "byJiO2r6Fup4uRL0lniok7t0AZGxbLx0lqvFBeRV3ivz+Gzrydrkta058c4GsoCp "
                + "KBJFpT0F+7g0Wj2yJlO6VWNyx1s/Ah05T9ZoUb04Ndl4uGgVw/Fn61QbEg22Vz73 "
                + "/K60rIhevCnOKt+MlwYi4nrUBDPDsm/qXa4vmhaGxzIxJNrPu5NJw6tXR6X5JwGe "
                + "1UTg6ufK3Ozusf/Ka+DkNgxHkAzV6xRjP6cPu9jUKd5ymT94Hz6jzJpU3q5/Brw1 "
                + "tIbdRwbKahWBY2USjDm+AOsvFGF1yqfqpNwDBV01aBL/aQ== -----END CERTIFICATE-----";
        m_certificateManager = createMock(CertificateManager.class);
        m_certificateManager.getSelfSigningAuthorityText();
        expectLastCall().andReturn(cert).times(1);
        replay(m_certificateManager);
    }

    public void testGenerateProfile41() throws Exception {

        SiteConfiguration app = new SiteConfiguration(phone41, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site.cfg");

        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile40() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone40, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile50() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone50, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-50.cfg");
        dumpXml(location.getReader(), System.out);
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile501() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone501, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-50.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile502() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone502, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-502.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }
    public void testGenerateProfile416() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone416, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-416.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile418() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone418, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-418.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }

    public void testGenerateProfile520() throws Exception {
        SiteConfiguration app = new SiteConfiguration(phone520, m_certificateManager);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-site-520.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());
        EasyMock.verify(m_certificateManager);
        expectedPhoneStream.close();
    }
}
