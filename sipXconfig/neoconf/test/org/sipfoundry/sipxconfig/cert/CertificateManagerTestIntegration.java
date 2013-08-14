/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class CertificateManagerTestIntegration extends IntegrationTestCase {
    private CertificateManagerImpl m_certificateManagerImpl;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("commserver/SeedLocations.sql");
    }

    public void testSaveCert() {
        m_certificateManagerImpl.checkSetup(AbstractCertificateCommon.DEFAULT_KEY_SIZE);
        String authority = "ca.example.org";
        Set<String> authorities = new HashSet<String>(m_certificateManagerImpl.getAuthorities());
        assertTrue(authorities.contains(authority));
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
