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
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Properties;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainManager;

/**
 * Backup provides Java interface to backup scripts
 */
public class WebCertificateManagerImpl implements WebCertificateManager {
    private static final Log LOG = LogFactory.getLog(WebCertificateManager.class);

    private static final String PROPERTIES_FILE = "webCert.properties";
    private static final String DOUBLE_QUOTES = "\"";
    private static final String READ_ERROR = "&msg.readError";
    private static final String WRITE_ERROR = "&msg.writeError";
    private static final String SCRIPT_ERROR = "&msg.scriptGenError";
    private static final String WORKDIR_FLAG = "--workdir";
    private static final String BLANK = " ";
    private static final String SCRIPT_EXCEPTION_MESSAGE = "Script finished with exit code ";
    private static final String RUNNING = "Running";
    private static final String DEFAULTS_FLAG = "--defaults";
    private static final String PARAMETERS_FLAG = "--parameters";
    private static final String GEN_SSL_KEYS_SH = "/ssl-cert/gen-ssl-keys.sh";
    private static final String WEB_ONLY = "--web-only";

    private String m_binDirectory;

    private String m_certDirectory;

    private String m_sslDirectory;

    private DomainManager m_domainManager;

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public void setCertDirectory(String certDirectory) {
        m_certDirectory = certDirectory;
    }

    public void setSslDirectory(String sslDirectory) {
        m_sslDirectory = sslDirectory;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public Properties loadCertPropertiesFile() {
        File propertiesFile = new File(m_certDirectory, PROPERTIES_FILE);
        try {
            FileInputStream propertiesStream = new FileInputStream(propertiesFile);
            Properties properties = new Properties();
            properties.load(propertiesStream);
            Enumeration<Object> propertiesEnum = properties.keys();
            while (propertiesEnum.hasMoreElements()) {
                String key = (String) propertiesEnum.nextElement();
                String value = properties.getProperty(key);
                value = StringUtils.strip(value, DOUBLE_QUOTES);
                properties.setProperty(key, value);
            }
            return properties;
        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            throw new UserException(READ_ERROR, propertiesFile.getPath());
        }
    }

    public void writeCertPropertiesFile(Properties properties) {
        File propertiesFile = new File(m_certDirectory, PROPERTIES_FILE);
        try {
            Enumeration<Object> propertiesEnum = properties.keys();
            while (propertiesEnum.hasMoreElements()) {
                String key = (String) propertiesEnum.nextElement();
                String value = properties.getProperty(key);
                value = DOUBLE_QUOTES + value + DOUBLE_QUOTES;
                properties.setProperty(key, value);
            }
            FileOutputStream propertiesStream = new FileOutputStream(propertiesFile);
            properties.store(propertiesStream, null);
        } catch (IOException e) {
            throw new UserException(WRITE_ERROR, propertiesFile.getPath());
        }
    }

    public String readCSRFile() {
        File csrFile = new File(m_certDirectory, getDomainName() + "-web.csr");
        try {
            return FileUtils.readFileToString(csrFile, "US-ASCII");
        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            throw new UserException(READ_ERROR, csrFile.getPath());
        }
    }

    public void generateCSRFile() {
        try {
            Runtime runtime = Runtime.getRuntime();
            String[] cmdLine = new String[] {
                m_binDirectory + GEN_SSL_KEYS_SH, "--csr", WEB_ONLY, DEFAULTS_FLAG, PARAMETERS_FLAG,
                PROPERTIES_FILE, WORKDIR_FLAG, m_certDirectory
            };
            Process proc = runtime.exec(cmdLine);
            LOG.debug("Executing: " + StringUtils.join(cmdLine, BLANK));
            proc.waitFor();
            if (proc.exitValue() != 0) {
                throw new UserException(SCRIPT_ERROR, SCRIPT_EXCEPTION_MESSAGE + proc.exitValue());
            }
        } catch (IOException e) {
            throw new UserException(SCRIPT_ERROR, e.getMessage());
        } catch (InterruptedException e) {
            throw new UserException(SCRIPT_ERROR, e.getMessage());
        }
    }

    public File getCRTFile() {
        return new File(m_certDirectory, getDomainName() + "-web.crt");
    }

    public void writeCRTFile(String crt) {
        File crtFile = getCRTFile();
        try {
            FileUtils.writeStringToFile(crtFile, crt);
        } catch (IOException e) {
            throw new UserException(WRITE_ERROR, crtFile.getPath());
        }
    }

    public void copyKeyAndCertificate() {
        File sourceCertificate = getCRTFile();
        if (!sourceCertificate.exists()) {
            return;
        }

        File sourceKey = new File(m_certDirectory, getDomainName() + "-web.key");
        if (!sourceKey.exists()) {
            return;
        }

        try {
            Runtime runtime = Runtime.getRuntime();
            String[] cmdLine = new String[] {
                m_binDirectory + GEN_SSL_KEYS_SH, WORKDIR_FLAG, m_certDirectory,
                "--pkcs", WEB_ONLY, DEFAULTS_FLAG, PARAMETERS_FLAG,
                PROPERTIES_FILE,
            };
            Process proc = runtime.exec(cmdLine);
            LOG.debug(RUNNING + StringUtils.join(cmdLine, BLANK));
            proc.waitFor();
            if (proc.exitValue() != 0) {
                throw new UserException(SCRIPT_ERROR, SCRIPT_EXCEPTION_MESSAGE + proc.exitValue());
            }
            File destinationCertificate = new File(m_sslDirectory, "ssl-web.crt");
            File destinationKey = new File(m_sslDirectory, "ssl-web.key");
            FileUtils.copyFile(sourceCertificate, destinationCertificate);
            FileUtils.copyFile(sourceKey, destinationKey);
            File sourceKeyStore = new File(m_certDirectory, getDomainName() + "-web.keystore");
            File destinationKeyStore = new File(m_sslDirectory, "ssl-web.keystore");
            FileUtils.copyFile(sourceKeyStore, destinationKeyStore);
            File sourcePkcsKeyStore = new File(m_certDirectory, getDomainName() + "-web.p12");
            File destinationPkcsKeyStore = new File(m_sslDirectory, "ssl-web.p12");
            FileUtils.copyFile(sourcePkcsKeyStore, destinationPkcsKeyStore);
        } catch (Exception e) {
            throw new UserException("&msg.copyError");
        }
    }

    private String getDomainName() {
        return m_domainManager.getDomain().getName();
    }
}
