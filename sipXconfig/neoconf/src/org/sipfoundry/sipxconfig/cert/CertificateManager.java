/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import java.util.List;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

/*
 * CRUD operations for certificate authorities, certificates and private keys
 */
public interface CertificateManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("certificates");

    public CertificateSettings getSettings();

    public void saveSettings(CertificateSettings settings);

    /**
     * No key, must be based on a return signed CSR from trusted authority
     */
    public void setThirdPartyTrustedWebCertificate(String cert);

    public void setSelfSignedWebCertificate(String cert, String key);

    /**
     * Do not support third party trusted authority signed certificate yet. No particular
     * reason other than no one has requested this yet.
     */
    public void setCommunicationsCertificate(String cert, String key, String authority);

    public String getWebCertificate();

    public String getWebPrivateKey();

    public String getCommunicationsCertificate();

    public String getCommunicationsPrivateKey();

    public List<String> getAuthorities();

    public String getAuthorityCertificate(String authority);

    public String getAuthorityKey(String authority);

    public void addTrustedAuthority(String authority, String cert);

    public void deleteTrustedAuthority(String authority);
}
