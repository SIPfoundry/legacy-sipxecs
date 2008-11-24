/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainConfiguration;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

public class InitialConfig {
    private static final String INITIAL_CONFIG = "/initial-config";
    private static final Log LOG = LogFactory.getLog(InitialConfig.class);

    private String m_tmpDirectory;
    private String m_binDirectory;
    private DomainConfiguration m_locationConfiguration;
    private DomainManager m_domainManager;

    /**
     * Set the directory where the initial-config script is located.  Note
     * that despite the name, this may not acutally be the "bin" dir
     */
    @Required
    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    @Required
    public void setTmpDirectory(String tmpDirectory) {
        m_tmpDirectory = tmpDirectory;
    }

    @Required
    public void setLocationConfiguration(DomainConfiguration locationConfiguration) {
        m_locationConfiguration = locationConfiguration;
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public String getTmpDirectory() {
        return m_tmpDirectory;
    }

    public InputStream getArchiveStream(String location) throws IOException {
        //generate domain-config for locations
        m_locationConfiguration.generate(m_domainManager.getDomain(), m_domainManager
                .getAuthorizationRealm(), location, m_domainManager.getExistingLocalization()
                .getLanguage(), m_domainManager.getAlarmServerUrl());

        String domainConfigContent = m_locationConfiguration.getFileContent();
        File domainConfigFile = new File(m_tmpDirectory + "/domain-config");
        domainConfigFile.createNewFile();
        BufferedWriter writer = new BufferedWriter(new FileWriter(domainConfigFile));
        writer.write(domainConfigContent);
        writer.close();
        //create the archive
        createArchive(location);
        InputStream stream = new FileInputStream(new File(m_tmpDirectory + INITIAL_CONFIG + "/"
                + location + ".tar.gz"));
        return stream;
    }

    private void createArchive(String locationName) {
        try {
            String[] cmdLine = new String[] {
                m_binDirectory + INITIAL_CONFIG, locationName
            };
            ProcessBuilder pb = new ProcessBuilder(cmdLine);
            java.lang.Process proc = pb.directory(new File(m_tmpDirectory)).start();
            LOG.debug("Executing: " + StringUtils.join(cmdLine, " "));
            proc.waitFor();

            if (proc.exitValue() != 0) {
                throw new UserException(false, "Script finished with exit code "
                        + proc.exitValue());
            }
        } catch (IOException e) {
            throw new UserException(false, e.getMessage());
        } catch (InterruptedException e) {
            throw new UserException(false, e.getMessage());
        }
    }
}
