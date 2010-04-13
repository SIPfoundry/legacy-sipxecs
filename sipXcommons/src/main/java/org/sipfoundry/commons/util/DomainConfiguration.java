/*
 *  Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Properties;

import org.apache.log4j.Logger;

/**
 * A class that represents the domain-config file.
 * This class is "read-only".
 *
 * @author C.Beeton
 *
 */
public class DomainConfiguration {
    private static String m_domainConfigFile;
    private static String m_sipDomainName;
    private static String m_sipRealm;
    private static String m_sharedSecret;

    private static Logger logger = Logger.getLogger(DomainConfiguration.class);

    public DomainConfiguration(String domainConfigFilename) {
        m_domainConfigFile = domainConfigFilename;
        try {
            Properties domainConfig = new Properties();
            File domainConfigFile = new File(m_domainConfigFile);
            logger.info("Attempting to load initial domain-config from " + domainConfigFile.getParentFile().getPath()
                    + "):");
            InputStream domainConfigInputStream = new FileInputStream(domainConfigFile);
            domainConfig.load(domainConfigInputStream);
            parseDomainConfig(domainConfig);
        } catch (Exception ex) {
            logger.error("Error loading domain-config", ex);
        }
    }

    public static String getSipRealm() {
        return m_sipRealm;
    }

    public static String getSipDomainName() {
        return m_sipDomainName;
    }

    public static String getSharedSecret() {
        return m_sharedSecret;
    }

    private static void setSipRealm(String sipRealm) {
        m_sipRealm = sipRealm;
    }

    private static void setSipDomainName(String sipDomainName) {
        m_sipDomainName = sipDomainName;
    }

    private static void setSharedSecret(String sharedSecret) {
        m_sharedSecret = sharedSecret;
    }

    private static void parseDomainConfig(Properties domainConfig) {
        setSipRealm(domainConfig.getProperty("SIP_REALM"));
        setSipDomainName(domainConfig.getProperty("SIP_DOMAIN_NAME"));
        setSharedSecret(domainConfig.getProperty("SHARED_SECRET"));
    }


}
