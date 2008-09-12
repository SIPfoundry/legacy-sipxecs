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
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Properties;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainManager;

/**
 * Backup provides Java interface to backup scripts
 */
public class WebCertificateManagerImpl implements WebCertificateManager {

    private static final String PROPERTIES_FILE = "webCert.properties";
    private static final String DOUBLE_QUOTES = "\"";
    private static final String READ_ERROR = "msg.readError";
    private static final String WRITE_ERROR = "msg.writeError";
    private static final String SCRIPT_ERROR = "msg.scriptGenError";
    private static final String COPY_ERROR = "msg.copyError";

    private String m_binDirectory;

    private String m_certDirectory;

    private String m_sslDirectory;

    private DomainManager m_domainManager;

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String getCertDirectory() {
        return m_certDirectory;
    }

    public void setCertDirectory(String certDirectory) {
        m_certDirectory = certDirectory;
    }

    public String getSslDirectory() {
        return m_sslDirectory;
    }

    public void setSslDirectory(String sslDirectory) {
        m_sslDirectory = sslDirectory;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public Properties loadCertPropertiesFile() {
        String path = getCertDirectory() + File.separator + PROPERTIES_FILE;
        try {
            File propertiesFile = new File(path);
            FileInputStream propertiesStream = new FileInputStream(propertiesFile);
            Properties properties = new Properties();
            properties.load(propertiesStream);
            Enumeration<Object> propertiesEnum = properties.keys();
            while (propertiesEnum.hasMoreElements()) {
                String key = (String) propertiesEnum.nextElement();
                String value = properties.getProperty(key);
                value = value.replace(DOUBLE_QUOTES, "");
                properties.setProperty(key, value);
            }
            return properties;
        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            throw new UserException(false, READ_ERROR, path);
        }
    }

    public String readCSRFile() {
        String path = getCertDirectory() + File.separator + getDomainName() + "-web.csr";
        try {
            File csrFile = new File(path);
            FileReader csrReader = new FileReader(csrFile);
            char[] csrArray = new char[(int) csrFile.length()];
            csrReader.read(csrArray);
            return new String(csrArray);
        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            throw new UserException(false, READ_ERROR, path);
        }
    }

    public void writeCertPropertiesFile(Properties properties) {
        String path = getCertDirectory() + File.separator + PROPERTIES_FILE;
        try {
            Enumeration<Object> propertiesEnum = properties.keys();
            while (propertiesEnum.hasMoreElements()) {
                String key = (String) propertiesEnum.nextElement();
                String value = properties.getProperty(key);
                value = DOUBLE_QUOTES + value + DOUBLE_QUOTES;
                properties.setProperty(key, value);
            }
            File propertiesFile = new File(path);
            FileOutputStream propertiesStream = new FileOutputStream(propertiesFile);
            properties.store(propertiesStream, null);
        } catch (Exception e) {
            throw new UserException(false, WRITE_ERROR, path);
        }
    }

    public void generateCSRFile() {
        try {
            String path = getBinDirectory() + File.separator + "ssl-cert" + File.separator;
            Runtime runtime = Runtime.getRuntime();
            StringBuilder builder = new StringBuilder();
            builder.append("/bin/bash ").append("./gen-ssl-keys.sh ").append("--csr ").append(
                    "--web-only ").append("--defaults ").append(
                    "--parameters webCert.properties ").append("--workdir ").append(
                    getCertDirectory());
            Process proc = runtime.exec(builder.toString(), null, new File(path));
            proc.waitFor();
            if (proc.exitValue() != 0) {
                throw new UserException(false, SCRIPT_ERROR, "Script finished with exit code "
                        + proc.exitValue());
            }
        } catch (Exception e) {
            throw new UserException(false, SCRIPT_ERROR, e.getMessage());
        }
    }

    public String getCRTFilePath() {
        return getCertDirectory() + File.separator + getDomainName() + "-web.crt";
    }

    public void writeCRTFile(String crt) {
        String path = getCRTFilePath();
        try {
            FileWriter writer = new FileWriter(path);
            writer.write(crt);
            writer.flush();
            writer.close();
        } catch (Exception e) {
            throw new UserException(false, WRITE_ERROR, path);
        }
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }

    public void copyKeyAndCertificate() {
        File sourceCertificate = new File(getCRTFilePath());
        File sourceKey = new File(getCertDirectory() + File.separator + getDomainName()
                + "-web.key");
        if (sourceCertificate.exists() && sourceKey.exists()) {
            try {
                File destinationCertificate = new File(getSslDirectory() + File.separator
                        + "ssl-web.crt");
                File destinationKey = new File(getSslDirectory() + File.separator + "ssl-web.key");

                // copy the certificate
                InputStream inCertificate = new FileInputStream(sourceCertificate);
                OutputStream outCertificate = new FileOutputStream(destinationCertificate);
                byte[] buf = new byte[1024];
                int len;
                while ((len = inCertificate.read(buf)) > 0) {
                    outCertificate.write(buf, 0, len);
                }
                inCertificate.close();
                outCertificate.close();

                // copy the key
                InputStream inKey = new FileInputStream(sourceKey);
                OutputStream outKey = new FileOutputStream(destinationKey);
                buf = new byte[1024];
                while ((len = inKey.read(buf)) > 0) {
                    outKey.write(buf, 0, len);
                }
                inKey.close();
                outKey.close();
            } catch (Exception e) {
                throw new UserException(false, COPY_ERROR);
            }
        }
    }
}
