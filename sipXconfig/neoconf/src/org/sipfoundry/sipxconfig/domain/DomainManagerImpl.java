/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.localization.Localization;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.springframework.dao.support.DataAccessUtils;

public abstract class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager {
    private static final String DOMAIN_CONFIG_ERROR = "Unable to load initial domain-config file.";

    private static final Log LOG = LogFactory.getLog(DomainManagerImpl.class);
    private static final String SIP_DOMAIN_NAME = "SIP_DOMAIN_NAME";

    private String m_domainConfigFilename;

    private LocationsManager m_locationsManager;

    protected abstract DomainConfiguration createDomainConfiguration();

    protected abstract ServiceConfigurator getServiceConfigurator();

    /**
     * @return non-null unless test environment
     */
    public Domain getDomain() {
        Domain domain = getExistingDomain();
        if (domain == null) {
            throw new DomainNotInitializedException();
        }

        return domain;
    }

    public void saveDomain(Domain domain) {
        if (domain.isNew()) {
            Domain existing = getExistingDomain();
            if (existing != null) {
                getHibernateTemplate().delete(getDomain());
            }
        }
        getHibernateTemplate().saveOrUpdate(domain);
        getHibernateTemplate().flush();

        // As domain change is critical change, force to replicate
        // all affected services' configurations.
        getServiceConfigurator().replicateAllServiceConfig();
    }

    public void replicateDomainConfig(SipxReplicationContext replicationContext, Location location) {
        Domain existingDomain = getExistingDomain();
        if (existingDomain == null) {
            throw new DomainNotInitializedException();
        }
        DomainConfiguration domainConfiguration = createDomainConfiguration();
        String language = getExistingLocalization().getLanguage();
        domainConfiguration.generate(existingDomain, getConfigServerHostname(), language);

        replicationContext.replicate(location, domainConfiguration);
    }

    protected Domain getExistingDomain() {
        Collection<Domain> domains = getHibernateTemplate().findByNamedQuery("domain");
        return (Domain) DataAccessUtils.singleResult(domains);
    }

    public Localization getExistingLocalization() {
        List l = getHibernateTemplate().loadAll(Localization.class);
        return (Localization) DataAccessUtils.singleResult(l);
    }

    public List<DialingRule> getDialingRules() {
        List<DialingRule> rules;
        Domain d = getDomain();
        if (d.hasAliases()) {
            DialingRule domainRule = new DomainDialingRule(getDomain());
            rules = Collections.<DialingRule> singletonList(domainRule);
        } else {
            rules = Collections.<DialingRule> emptyList();
        }
        return rules;
    }

    /**
     * Initialize the first and single domain supported by sipX at the moment. When this method is
     * called, a new domain is created and it is populated with the data from the domain-config
     * file which is written by sipxecs-setup during system installation. If there is an error
     * accessing this file, sipxconfig will not operate correctly.
     *
     * If this is called multiple times, the data in the domain table in the database will be
     * overwritten by what is currently contained in the domain-config file
     */
    public void initializeDomain() {
        try {
            Properties domainConfig = new Properties();
            File domainConfigFile = new File(m_domainConfigFilename);
            LOG.info("Attempting to load initial domain-config from " + domainConfigFile.getParentFile().getPath()
                    + "):");
            InputStream domainConfigInputStream = new FileInputStream(domainConfigFile);
            domainConfig.load(domainConfigInputStream);
            Domain domain = new Domain();
            parseDomainConfig(domain, domainConfig);
            domain.initSecret();
            saveDomain(domain);
        } catch (FileNotFoundException fnfe) {
            LOG.fatal(DOMAIN_CONFIG_ERROR, fnfe);
        } catch (IOException ioe) {
            LOG.fatal(DOMAIN_CONFIG_ERROR, ioe);
        }
    }

    private void parseDomainConfig(Domain domain, Properties domainConfig) {
        domain.setSipRealm(domainConfig.getProperty("SIP_REALM"));
        domain.setName(domainConfig.getProperty(SIP_DOMAIN_NAME));
        domain.setSharedSecret(domainConfig.getProperty("SHARED_SECRET"));
        domain.setAliases(getAlliasesFromDomainConfig(domainConfig));
    }

    public String getAuthorizationRealm() {
        return getDomain().getSipRealm();
    }

    public void setDomainConfigFilename(String domainConfigFilename) {
        m_domainConfigFilename = domainConfigFilename;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    private String getConfigServerHostname() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
    }

    public String getSharedSecret() {
        return getDomain().getSharedSecret();
    }

    private Set<String> getAlliasesFromDomainConfig(Properties domainConfig) {
        Set<String> aliases = new LinkedHashSet<String>();
        String[] domainConfigAliases = StringUtils.split(domainConfig.getProperty("SIP_DOMAIN_ALIASES"),
                DomainConfiguration.SEPARATOR_CHAR);
        if (domainConfigAliases != null) {
            for (String alias : domainConfigAliases) {
                if (!alias.equals(domainConfig.getProperty(SIP_DOMAIN_NAME))) {
                    aliases.add(alias);
                }
            }
        }

        return aliases;
    }
}
