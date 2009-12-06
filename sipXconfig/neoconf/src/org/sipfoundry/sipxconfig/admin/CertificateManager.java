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

    String readCSRFile();

    void writeCertPropertiesFile(Properties properties);

    void generateCSRFile();

    File getCRTFile();

    void writeCRTFile(String crt);

    void copyKeyAndCertificate();

    boolean validateCertificate(File file);

    String showCertificate(File file);

    public void rehashCertificates();

    public File getCAFile(String fileName);

    public File getCATmpFile(String fileName);

    void deleteCRTAuthorityTmpDirectory();

    void copyCRTAuthority();

    Set<CertificateDecorator> listCertificates();

    void deleteCA(CertificateDecorator cert);

    void deleteCAs(Collection<CertificateDecorator> listCert);
}
