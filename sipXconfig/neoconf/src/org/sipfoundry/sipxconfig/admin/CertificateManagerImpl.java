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
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Enumeration;
import java.util.Properties;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.io.FileUtils.copyDirectory;

/**
 * Backup provides Java interface to backup scripts
 */
public class CertificateManagerImpl implements CertificateManager {
    private static final Log LOG = LogFactory.getLog(CertificateManager.class);

    private static final String PROPERTIES_FILE = "webCert.properties";
    private static final String DOUBLE_QUOTES = "\"";
    private static final String READ_ERROR = "&msg.readError";
    private static final String WRITE_ERROR = "&msg.writeError";
    private static final String SCRIPT_ERROR = "&msg.scriptGenError";
    private static final String TMP = "tmp";
    private static final String WORKDIR_FLAG = "--workdir";
    private static final String BLANK = " ";
    private static final String SCRIPT_EXCEPTION_MESSAGE = "Script finished with exit code ";
    private static final String RUNNING = "Running";
    private static final String DEFAULTS_FLAG = "--defaults";
    private static final String PARAMETERS_FLAG = "--parameters";
    private static final String GEN_SSL_KEYS_SH = "/gen-ssl-keys.sh";
    private static final String CA_REHASH = "/ca_rehash";
    private static final String KEYSTOREGEN_SH = "/sipxkeystoregen";
    private static final String CHECK_CERT = "/check-cert.sh";
    private static final String WEB_ONLY = "--web-only";

    private String m_binCertDirectory;

    private String m_certDirectory;

    private String m_sslDirectory;

    private String m_sslAuthDirectory;

    private String m_certdbDirectory;

    private String m_libExecDirectory;

    private LocationsManager m_locationsManager;

    private SipxProcessContext m_processContext;

    private SipxServiceManager m_sipxServiceManager;

    public void setBinCertDirectory(String binCertDirectory) {
        m_binCertDirectory = binCertDirectory;
    }

    public void setCertDirectory(String certDirectory) {
        m_certDirectory = certDirectory;
    }

    public void setSslDirectory(String sslDirectory) {
        m_sslDirectory = sslDirectory;
    }

    public void setLibExecDirectory(String libExecDirectory) {
        m_libExecDirectory = libExecDirectory;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
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

    public String readCSRFile(String serverName) {
        File csrFile = new File(m_certDirectory, serverName + "-web.csr");
        try {
            return FileUtils.readFileToString(csrFile, "US-ASCII");
        } catch (FileNotFoundException e) {
            return null;
        } catch (IOException e) {
            throw new UserException(READ_ERROR, csrFile.getPath());
        }
    }

    private String[] getGenerateCSRFileCommand() {
        String[] cmdLine = new String[] {
            m_binCertDirectory + GEN_SSL_KEYS_SH, "--csr", WEB_ONLY, DEFAULTS_FLAG, PARAMETERS_FLAG,
            PROPERTIES_FILE, WORKDIR_FLAG, m_certDirectory
        };
        return cmdLine;
    }

    private String[] getValidateCertFileCommand(File file) {
        String[] cmdLine = new String[] {
            m_binCertDirectory + CHECK_CERT, "--certificate-authority ", file.getPath()};
        return cmdLine;
    }

    private String[] getShowCertFileCommand(File file) {
        String[] cmdLine = new String[] {
            m_binCertDirectory + GEN_SSL_KEYS_SH, "--show-cert", file.getPath()};
        return cmdLine;
    }

    private String[] getHashCertsCommand() {
        String[] cmdLine = new String[] {
            m_binCertDirectory + CA_REHASH};
        return cmdLine;
    }
    private String[] getKeyStoreGenCommand() {
        String[] cmdLine = new String[] {
            m_libExecDirectory + KEYSTOREGEN_SH};
        return cmdLine;
    }

    public File getCAFile(String fileName) {
        return new File(m_sslAuthDirectory + File.separator + fileName);
    }

    public File getCATmpFile(String fileName) {
        return new File(getCATmpDir() + File.separator + fileName);
    }

    private String getCATmpDir() {
        try {
            FileUtils.forceMkdir(new File(m_sslAuthDirectory + File.separator + TMP));
            return m_sslAuthDirectory + File.separator + TMP;
        } catch (IOException ex) {
            throw new UserException("&error.temp");
        }
    }

    public void setSslAuthDirectory(String sslAuthDirectory) {
        m_sslAuthDirectory = sslAuthDirectory;
    }

    private InputStream runCommand(String[] command) {
        try {
            Runtime runtime = Runtime.getRuntime();

            Process proc = runtime.exec(command);
            LOG.debug("Executing: " + StringUtils.join(command, BLANK));
            proc.waitFor();
            if (proc.exitValue() != 0) {
                throw new ScriptExitException(proc.exitValue());
            }
            return proc.getInputStream();
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    public void generateCSRFile() {
        try {
            runCommand(getGenerateCSRFileCommand());
        } catch (RuntimeException e) {
            throw new UserException(SCRIPT_ERROR, e.getMessage());
        }
    }

    public boolean validateCertificate(File file) {
        try {
            runCommand(getValidateCertFileCommand(file));
            return true;
        } catch (ScriptExitException ex) {
            deleteCRTAuthorityTmpDirectory();
            return false;
        } catch (RuntimeException ex) {
            deleteCRTAuthorityTmpDirectory();
            throw new UserException("&error.validate", file.getName());
        }
    }

    public String showCertificate(File file) {
        try {
            InputStream inputStream = runCommand(getShowCertFileCommand(file));
            return IOUtils.toString(inputStream);
        } catch (Exception ex) {
            throw new UserException("&error.show", file.getName());
        }
    }

    public void rehashCertificates() {
        try {
            runCommand(getHashCertsCommand());
        } catch (RuntimeException ex) {
            throw new UserException("&error.rehash");
        }
    }

    public void generateKeyStores() {
        try {
            runCommand(getKeyStoreGenCommand());
            //Mark required services for restart
            markServicesForRestart();
        } catch (RuntimeException ex) {
            throw new UserException("&error.regenstore");
        }
    }

    private void markServicesForRestart() {
        SipxConfigService configService = (SipxConfigService) m_sipxServiceManager.
            getServiceByBeanId(SipxConfigService.BEAN_ID);
        SipxBridgeService bridgeService = (SipxBridgeService) m_sipxServiceManager.
            getServiceByBeanId(SipxBridgeService.BEAN_ID);
        m_processContext.markServicesForRestart(Arrays.asList(configService, bridgeService));
    }

    public File getCRTFile() {
        return new File(m_certDirectory, getPrimaryServerFqdn() + "-web.crt");
    }

    public void deleteCA(CertificateDecorator cert) {
        File certdbFile = new File(m_certdbDirectory + File.separatorChar + cert.getFileName());
        if (certdbFile.exists()) {
            throw new UserException("&error.delete.default.ca", cert.getFileName());
        }

        Collection<File> files = FileUtils.listFiles(new File(m_sslAuthDirectory), null, false);
        Collection<File> filesToDelete = new ArrayList<File>();
        try {
            for (File file : files) {
                if (StringUtils.equals(file.getCanonicalFile().getName(), cert.getFileName())) {
                    filesToDelete.add(getCAFile(file.getName()));
                }
            }
            for (File file : filesToDelete) {
                FileUtils.deleteQuietly(file);
            }
        } catch (IOException ex) {
            throw new UserException("&error.delete.cert");
        }
    }

    public void deleteCAs(Collection<CertificateDecorator> listCert) {
        UserException userEx = null;
        for (CertificateDecorator certDecorator : listCert) {
            try {
                deleteCA(certDecorator);
            } catch (UserException ex) {
                userEx = ex;
            }
        }

        if (userEx != null) {
            throw userEx;
        }
    }

    public void copyCRTAuthority() {
        try {
            copyDirectory(new File(getCATmpDir()), new File(m_sslAuthDirectory));
        } catch (IOException ex) {
            throw new UserException("&error.keep");
        }
    }

    public Set<CertificateDecorator> listCertificates() {
        Set<CertificateDecorator> certificates = new TreeSet<CertificateDecorator>();
        Collection<File> files = FileUtils.listFiles(new File(m_sslAuthDirectory), null, false);
        CertificateDecorator certificateDecorator = null;

        for (File file : files) {
            try {
                file = file.getCanonicalFile();
            } catch (IOException ex) {
                continue;
            }
            certificateDecorator = new CertificateDecorator();
            certificateDecorator.setFileName(file.getName());
            certificates.add(certificateDecorator);
        }
        return certificates;
    }

    public void deleteCRTAuthorityTmpDirectory() {
        try {
            FileUtils.deleteDirectory(new File(getCATmpDir()));
        } catch (IOException ex) {
            LOG.error("Cannot delete temporary directory");
            //Temporary path cannot be deleted
        }
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

        File sourceKey = new File(m_certDirectory, getPrimaryServerFqdn() + "-web.key");
        if (!sourceKey.exists()) {
            return;
        }

        try {
            Runtime runtime = Runtime.getRuntime();
            String[] cmdLine = new String[] {
                m_binCertDirectory + GEN_SSL_KEYS_SH, WORKDIR_FLAG, m_certDirectory, "--pkcs", WEB_ONLY, DEFAULTS_FLAG,
                PARAMETERS_FLAG, PROPERTIES_FILE,
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
            File sourceKeyStore = new File(m_certDirectory, getPrimaryServerFqdn() + "-web.keystore");
            File destinationKeyStore = new File(m_sslDirectory, "ssl-web.keystore");
            FileUtils.copyFile(sourceKeyStore, destinationKeyStore);
            File sourcePkcsKeyStore = new File(m_certDirectory, getPrimaryServerFqdn() + "-web.p12");
            File destinationPkcsKeyStore = new File(m_sslDirectory, "ssl-web.p12");
            FileUtils.copyFile(sourcePkcsKeyStore, destinationPkcsKeyStore);
        } catch (Exception e) {
            throw new UserException("&msg.copyError");
        }
    }

    private String getPrimaryServerFqdn() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public void setCertdbDirectory(String certdbDirectory) {
        m_certdbDirectory = certdbDirectory;
    }

    public class ScriptExitException extends UserException {
        public ScriptExitException(int exitCode) {
            super("&error.script.exit.code", exitCode);
        }
    }

    @Required
    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

}
