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
import java.util.Properties;

public interface WebCertificateManager {
    Properties loadCertPropertiesFile();

    String readCSRFile();

    void writeCertPropertiesFile(Properties properties);

    void generateCSRFile();

    File getCRTFile();

    void writeCRTFile(String crt);

    void copyKeyAndCertificate();
}
