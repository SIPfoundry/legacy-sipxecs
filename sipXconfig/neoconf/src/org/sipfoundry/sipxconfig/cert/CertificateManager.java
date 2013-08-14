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
    public static final String COMM_CERT = "ssl";
    public static final String WEB_CERT = "ssl-web";
    public static final String CHAIN_CERT = "server-chain";
    public static final String CA_CERT = "ca-bundle";
    public static final String CRT = ".crt";

    public CertificateSettings getSettings();

    public void saveSettings(CertificateSettings settings);

    /**
     * No key, must be based on a return signed CSR from trusted authority
     */
    public void setWebCertificate(String cert);

    public void setWebCertificate(String cert, String key);

    /**
     * Do not support third party trusted authority signed certificate yet. No particular
     * reason other than no one has requested this yet.
     */
    public void setCommunicationsCertificate(String cert, String key);

    /**
     * No key, must be based on a return signed CSR from trusted authority
     */
    public void setCommunicationsCertificate(String cert);

    public String getWebCertificate();

    public String getWebPrivateKey();

    public String getCommunicationsCertificate();

    public String getCommunicationsPrivateKey();

    public List<String> getThirdPartyAuthorities();

    public List<String> getAuthorities();

    public String getAuthorityCertificate(String authority);

    public String getAuthorityKey(String authority);

    public void addTrustedAuthority(String authority, String cert);

    public void deleteTrustedAuthority(String authority);

    public void updateNamedCertificate(String name, String cert, String privateKey, String authority);

    public String getNamedPrivateKey(String name);

    public String getNamedCertificate(String name);

    public String getSelfSigningAuthority();

    public String getSelfSigningAuthorityText();

    public void rebuildSelfSignedData(int keySize);

    public void rebuildCommunicationsCert(int keySize);

    public void rebuildWebCert(int keySize);

    public String getChainCertificate();

    public void setChainCertificate(String cert);

    public String getCACertificate();

    public void setCACertificate(String cert);
}
