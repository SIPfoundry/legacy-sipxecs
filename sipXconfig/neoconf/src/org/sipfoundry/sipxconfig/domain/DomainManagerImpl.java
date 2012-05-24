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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.localization.Localization;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.dao.support.DataAccessUtils;

public class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager, SetupListener {
    private static DomainManagerImpl s_instance;
    private static final String DOMAIN_CONFIG_ERROR = "Unable to load initial domain-config file.";
    private static final Log LOG = LogFactory.getLog(DomainManagerImpl.class);
    private static final String SIP_DOMAIN_NAME = "SIP_DOMAIN_NAME";
    private Domain m_domain;
    private String m_domainConfigFilename;

    public DomainManagerImpl() {
        s_instance = this;
    }

    static Domain getDomainInstance() {
        return s_instance.getDomain();
    }

    /**
     * @return non-null unless test environment
     */
    public Domain getDomain() {
        Domain domain = getExistingDomain();
        if (domain == null) {
            initializeDomain();
            domain = getExistingDomain();
            if (domain == null) {
                throw new DomainNotInitializedException();
            }
        }

        return domain;
    }

    /**
     * convenience method. it helps reducing the number of mocks in tests
     * @return
     */
    public String getDomainName() {
        return getDomain().getName();
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

        // hmmm, should each feature that uses the domain name be watching for
        // DomainManger.FEATURE?
        // m_configManager.allFeaturesAffected();
        m_domain = null;
    }

    protected Domain getExistingDomain() {
        if (m_domain == null) {
            Collection<Domain> domains = getHibernateTemplate().findByNamedQuery("domain");
            m_domain = (Domain) DataAccessUtils.singleResult(domains);
        }
        return m_domain;
    }

    public Localization getExistingLocalization() {
        List l = getHibernateTemplate().loadAll(Localization.class);
        return (Localization) DataAccessUtils.singleResult(l);
    }

    public List<DialingRule> getDialingRules(Location location) {
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
            saveDomain(domain);
            // Do not publish as there should not be a change
            // getDaoEventPublisher().publishSave(domain);
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

    public String getSharedSecret() {
        return getDomain().getSharedSecret();
    }

    private Set<String> getAlliasesFromDomainConfig(Properties domainConfig) {
        String[] domainConfigAliases = StringUtils.split(domainConfig.getProperty("SIP_DOMAIN_ALIASES"), ' ');
        Set<String> aliases = new LinkedHashSet<String>(Arrays.asList(domainConfigAliases));
        return aliases;
    }

    /**
     * For use in tests only.
     */
    public void setNullDomain() {
        Collection<Domain> domains = getHibernateTemplate().loadAll(Domain.class);
        if (!domains.isEmpty()) {
            getHibernateTemplate().deleteAll(domains);
            getDaoEventPublisher().publishDeleteCollection(domains);
            getHibernateTemplate().flush();
        }
        m_domain = null;
    }

    public void setDomainConfigFilename(String domainConfigFilename) {
        m_domainConfigFilename = domainConfigFilename;
    }

    @Override
    public void setup(SetupManager manager) {
        if (getDomain() == null) {
            initializeDomain();
        }
    }
}
