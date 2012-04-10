/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.domain.Domain;

public class CertificateConfig implements ConfigProvider {
    private CertificateManager m_certificateManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(CertificateManager.FEATURE)) {
            return;
        }

        File dir = manager.getGlobalDataDirectory();
        String sipCert = m_certificateManager.getCommunicationsCertificate();
        FileUtils.writeStringToFile(new File(dir, "ssl.crt"), sipCert);
        String sipKey = m_certificateManager.getCommunicationsPrivateKey();
        FileUtils.writeStringToFile(new File(dir, "ssl.key"), sipKey);
        String webCert = m_certificateManager.getWebCertificate();
        FileUtils.writeStringToFile(new File(dir, "ssl-web.crt"), webCert);
        String webKey = m_certificateManager.getWebPrivateKey();
        FileUtils.writeStringToFile(new File(dir, "ssl-web.key"), webKey);

        String domain = Domain.getDomain().getName();

        JavaKeyStore sslSip = new JavaKeyStore();
        sslSip.addKey(domain, sipCert, sipKey);
        sslSip.storeIfDifferent(new File(dir, "ssl.keystore"));

        JavaKeyStore sslWeb = new JavaKeyStore();
        sslWeb.addKey(domain, webCert, webKey);
        sslWeb.storeIfDifferent(new File(dir, "ssl-web.keystore"));

        File authDir = new File(dir, "authorities");
        authDir.mkdir();
        JavaKeyStore store = new JavaKeyStore();
        for (String authority : m_certificateManager.getAuthorities()) {
            String authCert = m_certificateManager.getAuthorityCertificate(authority);
            FileUtils.writeStringToFile(new File(authDir, authority + ".crt"), authCert);
            store.addAuthority(authority, authCert);
        }
        OutputStream authoritiesStore = null;
        try {
            store.storeIfDifferent(new File(dir, "authorities.jks"));
        } finally {
            IOUtils.closeQuietly(authoritiesStore);
        }
    }

    public void setCertificateManager(CertificateManager certificateManager) {
        m_certificateManager = certificateManager;
    }
}
