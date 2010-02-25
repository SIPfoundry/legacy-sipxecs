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
import java.security.cert.X509Certificate;
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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.SoftwareAdminApi;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxProvisionService;
import org.sipfoundry.sipxconfig.service.SipxRecordingService;
import org.sipfoundry.sipxconfig.service.SipxRestService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

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
    private static final String WEB_KEY = "-web.key";
    private static final String SSL_CERT = "ssl.crt";
    private static final String ERROR_CERT_VALIDATE = "&error.validate";
    private static final String ERROR_MSG_COPY = "&msg.copyError";
    private static final String ERROR_VALID = "&error.valid";
    private static final String EXTERNAL_KEY_BASED = "external-key-based";
    private static final String RESTART_SIPXECS = "restart";

    private String m_binCertDirectory;

    private String m_certDirectory;

    private String m_sslDirectory;

    private String m_sslAuthDirectory;

    private String m_certdbDirectory;

    private String m_libExecDirectory;

    private SipxProcessContext m_processContext;

    private SipxServiceManager m_sipxServiceManager;

    private SipxReplicationContext m_sipxReplicationContext;

    private ApiProvider<SoftwareAdminApi>  m_softwareAdminApiProvider;

    private LocationsManager m_locationsManager;

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

    private String[] getValidateCertFileCommand(File file, boolean isCA) {
        if (isCA) {
            return new String[] {
                m_binCertDirectory + CHECK_CERT, "--certificate-authority ", file.getPath()
            };
        } else {
            return new String[] {
                m_binCertDirectory + CHECK_CERT, file.getPath()
            };
        }

    }

    private String[] getShowCertFileCommand(File file) {
        String[] cmdLine = new String[] {
            m_binCertDirectory + GEN_SSL_KEYS_SH, "--show-cert", file.getPath()
        };
        return cmdLine;
    }

    private String[] getHashCertsCommand() {
        String[] cmdLine = new String[] {
            m_binCertDirectory + CA_REHASH
        };
        return cmdLine;
    }
    private String[] getKeyStoreGenCommand() {
        String[] cmdLine = new String[] {
            m_libExecDirectory + KEYSTOREGEN_SH
        };
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

    public boolean validateCertificateAuthority(File file) {
        try {
            runCommand(getValidateCertFileCommand(file, true));
            return true;
        } catch (ScriptExitException ex) {
            deleteCRTAuthorityTmpDirectory();
            return false;
        } catch (RuntimeException ex) {
            deleteCRTAuthorityTmpDirectory();
            throw new UserException(ERROR_CERT_VALIDATE, file.getName());
        }
    }

    public boolean validateCertificate(File file) {
        try {
            runCommand(getValidateCertFileCommand(file, false));
            return true;
        } catch (ScriptExitException ex) {
            return false;
        } catch (RuntimeException ex) {
            throw new UserException(ERROR_CERT_VALIDATE, file.getName());
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

    public void restartRemote() {
        // restart distributed systems in order to regenerate their keystore/truststore.
        try {
            Location[] locations = m_locationsManager.getLocations();
            for (Location location : locations) {
                if (!location.isPrimary()) {
                    SoftwareAdminApi api = m_softwareAdminApiProvider.getApi(location.getSoftwareAdminUrl());
                    api.exec(m_locationsManager.getPrimaryLocation().getFqdn(), RESTART_SIPXECS);
                }
            }
        } catch (XmlRpcRemoteException e) {
            LOG.error("Cannot restart remote locations", e);
        }
    }

    public void generateKeyStores() {
        try {
            runCommand(getKeyStoreGenCommand());
            // Mark required services for restart
            markServicesForRestart();
        } catch (RuntimeException ex) {
            throw new UserException("&error.regenstore");
        }
    }

    /**
     * Mark for restart only services from primary location.
     * Anyway, sipXecs from secondary locations
     * is restarted, so there is no need to mark services for restart for these locations
     */
    private void markServicesForRestart() {
        SipxService configService = m_sipxServiceManager
                .getServiceByBeanId(SipxConfigService.BEAN_ID);
        SipxService bridgeService = m_sipxServiceManager
                .getServiceByBeanId(SipxBridgeService.BEAN_ID);
        SipxService ivrService = m_sipxServiceManager
                .getServiceByBeanId(SipxIvrService.BEAN_ID);
        SipxService recordingService = m_sipxServiceManager
                .getServiceByBeanId(SipxRecordingService.BEAN_ID);
        SipxService imbotService = m_sipxServiceManager
                .getServiceByBeanId(SipxImbotService.BEAN_ID);
        SipxService openfireService = m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        SipxService provisionService = m_sipxServiceManager
                .getServiceByBeanId(SipxProvisionService.BEAN_ID);
        SipxService restService = m_sipxServiceManager
                .getServiceByBeanId(SipxRestService.BEAN_ID);

        Location primaryLocation = m_locationsManager.getPrimaryLocation();

        m_processContext.markServicesForRestart(primaryLocation,
                                   Arrays.asList(configService, bridgeService, ivrService,
                                                 recordingService, openfireService, imbotService,
                                                 provisionService, restService));
    }

    public File getCRTFile(String serverName) {
        return new File(m_certDirectory, serverName + "-web.crt");
    }

    public File getExternalCRTFile() {
        return getCRTFile(EXTERNAL_KEY_BASED);
    }

    public File getKeyFile(String serverName) {
        return new File(m_certDirectory, serverName + WEB_KEY);
    }


    public File getExternalKeyFile() {
        return getKeyFile(EXTERNAL_KEY_BASED);
    }

    public void deleteCA(CertificateDecorator cert) {
        if (isSystemGenerated(cert.getFileName())) {
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

    // check the issuer DN name against ssl.crt (always signed by sipx)
    private boolean isSystemGenerated(String fileName) {
        X509Certificate sslCrt = X509CertificateUtils.getX509Certificate(m_sslDirectory + File.separatorChar
                + SSL_CERT);
        X509Certificate currentCertificate = X509CertificateUtils.getX509Certificate(m_certdbDirectory
                + File.separatorChar + fileName);
        if (sslCrt != null && currentCertificate != null) {
            return StringUtils.equals(sslCrt.getIssuerDN().getName(), currentCertificate.getIssuerDN().getName());
        }
        return false;
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
        AnyFile anyFile = null;
        Location[] locations = m_locationsManager.getLocations();
        for (File file : new File(getCATmpDir()).listFiles()) {
            anyFile = new AnyFile();
            anyFile.setDirectory(m_sslAuthDirectory);
            anyFile.setName(file.getName());
            anyFile.setSourceFilePath(file.getAbsolutePath());
            for (Location location : locations) {
                //call replicate on each location so JobStatus page can show replication status on
                //each affected location
                m_sipxReplicationContext.replicate(location, anyFile);
            }
        }
        deleteCRTAuthorityTmpDirectory();
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
            certificateDecorator.setDirName(file.getParentFile().getPath());
            certificateDecorator.setSystemGenerated(isSystemGenerated(file.getName()));
            certificates.add(certificateDecorator);
        }
        return certificates;
    }

    public void deleteCRTAuthorityTmpDirectory() {
        try {
            FileUtils.deleteDirectory(new File(getCATmpDir()));
        } catch (IOException ex) {
            LOG.error("Cannot delete temporary certificate directory");
            // Temporary path cannot be deleted
        }
    }

    public void writeCRTFile(String crt, String server) {
        File crtFile = getCRTFile(server);

        try {
            FileUtils.writeStringToFile(getCRTFile(server), crt);
        } catch (IOException e) {
            throw new UserException(WRITE_ERROR, crtFile.getPath());
        }
    }

    public void writeExternalCRTFile(String crt) {
        writeCRTFile(crt, EXTERNAL_KEY_BASED);
    }

    public void writeKeyFile(String key) {
        File keyFile = getExternalKeyFile();
        try {
            FileUtils.writeStringToFile(keyFile, key);
        } catch (IOException e) {
            throw new UserException(WRITE_ERROR, keyFile.getPath());
        }
    }

    public void importKeyAndCertificate(String server, boolean isCsrBased) {
        File newCertificate = isCsrBased ? getCRTFile(server) : getExternalCRTFile();
        if (!newCertificate.exists()) {
            throw new UserException(ERROR_VALID);
        }

        File newKey = isCsrBased ? getKeyFile(server) :  getExternalKeyFile();
        if (!newKey.exists()) {
            throw new UserException(ERROR_VALID);
        }

        if (!validateCertificate(newCertificate)) {
            throw new UserException(ERROR_VALID);
        }

        File oldCertificate = new File(m_sslDirectory, "ssl-web.crt");
        File oldKey = new File(m_sslDirectory, "ssl-web.key");

        File backupCertificate = new File(m_sslDirectory, "ssl-web.oldcrt");
        File backupKey = new File(m_sslDirectory, "ssl-web.oldkey");

        try {
            if (isCsrBased) {
                Runtime runtime = Runtime.getRuntime();
                String[] cmdLine = new String[] {
                    m_binCertDirectory + GEN_SSL_KEYS_SH, WORKDIR_FLAG, m_certDirectory, "--pkcs", WEB_ONLY,
                    DEFAULTS_FLAG, PARAMETERS_FLAG, PROPERTIES_FILE,
                };
                Process proc = runtime.exec(cmdLine);
                LOG.debug(RUNNING + StringUtils.join(cmdLine, BLANK));
                proc.waitFor();
                if (proc.exitValue() != 0) {
                    throw new UserException(SCRIPT_ERROR, SCRIPT_EXCEPTION_MESSAGE + proc.exitValue());
                }
            }

            FileUtils.copyFile(oldCertificate, backupCertificate);
            FileUtils.copyFile(oldKey, backupKey);

            FileUtils.copyFile(newCertificate, oldCertificate);
            FileUtils.copyFile(newKey, oldKey);
        } catch (Exception ex) {
            throw new UserException(ERROR_MSG_COPY);
        }

        try {
            generateKeyStores();
        } catch (UserException userException) {
            try {
                FileUtils.copyFile(backupCertificate, oldCertificate);
                FileUtils.copyFile(backupKey, oldKey);

                backupCertificate.delete();
                backupKey.delete();

            } catch (Exception ex) {
                throw new UserException(ERROR_MSG_COPY);
            }

            throw userException;
        }

        backupCertificate.delete();
        backupKey.delete();
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

    @Required
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Required
    public void setSoftwareAdminApiProvider(ApiProvider<SoftwareAdminApi>  softwareAdminApiProvider) {
        m_softwareAdminApiProvider = softwareAdminApiProvider;
    }

    @Required
    public void setLocationsManager(LocationsManager  locationsManager) {
        m_locationsManager = locationsManager;
    }
}
