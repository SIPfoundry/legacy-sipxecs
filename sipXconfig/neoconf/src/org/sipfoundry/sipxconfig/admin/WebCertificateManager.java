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

import java.util.Properties;

public interface WebCertificateManager {
    public static final String CONTEXT_BEAN_NAME = "webCertificateManager";

    public Properties loadCertPropertiesFile();

    public String readCSRFile();

    public void writeCertPropertiesFile(Properties properties);

    public void generateCSRFile();

    public String getCRTFilePath();

    public void writeCRTFile(String crt);

    public String getDomainName();

    public void copyKeyAndCertificate();
}
