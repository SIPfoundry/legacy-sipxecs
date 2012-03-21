/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.IOException;
import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class CertificateManagerIntegrationTest extends IntegrationTestCase {
    private CertificateManagerImpl m_certificateManagerImpl;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");        
    }
    
    public void testSaveCert() throws IOException {
        m_certificateManagerImpl.checkSetup();
        String authority = "ca.example.org";
        List<String> authorities = m_certificateManagerImpl.getAuthorities();
        assertEquals(String.format("[%s]", authority), authorities.toString());
        assertNotNull(m_certificateManagerImpl.getWebCertificate());
        assertNotNull(m_certificateManagerImpl.getWebPrivateKey());
        assertNotNull(m_certificateManagerImpl.getCommunicationsCertificate());
        assertNotNull(m_certificateManagerImpl.getCommunicationsPrivateKey());
        assertNotNull(m_certificateManagerImpl.getAuthorityCertificate(authority));
        assertNotNull(m_certificateManagerImpl.getAuthorityKey(authority));
    }

    public void setCertificateManagerImpl(CertificateManagerImpl certificateManager) {
        m_certificateManagerImpl = certificateManager;
    }    
}
