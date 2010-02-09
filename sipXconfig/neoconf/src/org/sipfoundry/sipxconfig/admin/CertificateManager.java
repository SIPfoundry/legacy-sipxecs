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
import java.util.Collection;
import java.util.Properties;
import java.util.Set;


public interface CertificateManager {
    Properties loadCertPropertiesFile();

    String readCSRFile(String serverName);

    void writeCertPropertiesFile(Properties properties);

    void generateCSRFile();

    File getCRTFile(String server);

    File getExternalCRTFile();

    void writeCRTFile(String crt, String server);

    void writeExternalCRTFile(String crt);

    File getKeyFile(String server);

    File getExternalKeyFile();

    void writeKeyFile(String key);

    void importKeyAndCertificate(String server, boolean isCsrBased);

    boolean validateCertificate(File file);

    boolean validateCertificateAuthority(File file);

    String showCertificate(File file);

    public void rehashCertificates();

    public void generateKeyStores();

    public File getCAFile(String fileName);

    public File getCATmpFile(String fileName);

    void deleteCRTAuthorityTmpDirectory();

    void copyCRTAuthority();

    Set<CertificateDecorator> listCertificates();

    void deleteCA(CertificateDecorator cert);

    void deleteCAs(Collection<CertificateDecorator> listCert);
}
