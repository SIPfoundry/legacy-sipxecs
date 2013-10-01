/*
 *  Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.util;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
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
    private static final Logger logger = Logger.getLogger(DomainConfiguration.class);

    private final String m_domainConfigFile;
    private String m_sipDomainName;
    private String m_sipRealm;
    private String m_sharedSecret;

    public DomainConfiguration(String domainConfigFilename) {
        m_domainConfigFile = domainConfigFilename;
        InputStream domainConfigInputStream = null;
        try {
            Properties domainConfig = new Properties();
            File domainConfigFile = new File(m_domainConfigFile);
            logger.info("Attempting to load initial domain-config from " + domainConfigFile.getParentFile().getPath()
                    + "):");
            domainConfigInputStream = new FileInputStream(domainConfigFile);
            domainConfig.load(domainConfigInputStream);
            parseDomainConfig(domainConfig);
        } catch (Exception ex) {
            logger.error("Error loading domain-config", ex);
        } finally {
            // IOUtils.closeQuietly(domainConfigInputStream);
            // closing old fashioned way for 4.4, didn't want to introduce dep
            if (domainConfigInputStream != null) {
                try {
                    domainConfigInputStream.close();
                } catch (IOException ignore) {
                    logger.error("Error closing stream", ignore);
                }
            }
	}
    }

    public String getSipRealm() {
        return m_sipRealm;
    }

    public String getSipDomainName() {
        return m_sipDomainName;
    }

    public String getSharedSecret() {
        return m_sharedSecret;
    }

    private void setSipRealm(String sipRealm) {
        m_sipRealm = sipRealm;
    }

    private void setSipDomainName(String sipDomainName) {
        m_sipDomainName = sipDomainName;
    }

    private void setSharedSecret(String sharedSecret) {
        m_sharedSecret = sharedSecret;
    }

    private void parseDomainConfig(Properties domainConfig) {
        setSipRealm(domainConfig.getProperty("SIP_REALM"));
        setSipDomainName(domainConfig.getProperty("SIP_DOMAIN_NAME"));
        setSharedSecret(domainConfig.getProperty("SHARED_SECRET"));
    }


}
