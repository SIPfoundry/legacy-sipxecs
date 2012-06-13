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
    private volatile Domain m_domain;
    private String m_domainConfigFilename;

    public DomainManagerImpl() {
        s_instance = this;
    }

    static Domain getDomainInstance() {
        return s_instance.getDomain();
    }

    public Domain getEditableDomain() {
        try {
            Domain d = getDomain().clone();
            getHibernateTemplate().evict(d);
            return d;
        } catch (CloneNotSupportedException impossible) {
            throw new IllegalStateException(impossible);
        }
    }


    /**
     * @return non-null unless test environment
     */
    public Domain getDomain() {
        Domain d = m_domain;
        if (d == null) {
            // double-check lock with local references as proper guard and additional
            // local references to comply w/checkstyle. See
            //   http://en.wikipedia.org/wiki/Double-checked_locking
            synchronized (this) {
                Domain d2 = m_domain;
                if (d2 == null) {
                    d2 = loadDomainFromDb();
                    if (d2 == null) {
                        d2 = loadDomainFromConfig();
                        getHibernateTemplate().saveOrUpdate(d2);
                        getHibernateTemplate().flush();
                        d2 = loadDomainFromDb();
                        if (d2 == null) {
                            throw new DomainNotInitializedException();
                        }
                    }
                    m_domain = d2;
                }
            }
        }

        return m_domain;
    }

    private Domain loadDomainFromDb() {
        Collection<Domain> domains = getHibernateTemplate().findByNamedQuery("domain");
        return (Domain) DataAccessUtils.singleResult(domains);
    }

    /**
     * convenience method. it helps reducing the number of mocks in tests
     * @return
     */
    public String getDomainName() {
        return getDomain().getName();
    }

    public void saveDomain(Domain domain) {
        if (domain == m_domain) {
            throw new IllegalStateException("Cannnot edit global domain, call DomainManager.getEditableDomain");
        }
        if (domain.isNew()) {
            throw new IllegalStateException("Cannnot save more than one domain");
        }
        if (!domain.getId().equals(m_domain.getId())) {
            throw new IllegalStateException("Cannnot change domain id");
        }
        Domain d = getHibernateTemplate().merge(domain);
        getHibernateTemplate().update(d);
        getHibernateTemplate().flush();
        m_domain = null;
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
    public Domain loadDomainFromConfig() {
        Domain domain = new Domain();
        try {
            Properties domainConfig = new Properties();
            File domainConfigFile = new File(m_domainConfigFilename);
            LOG.info("Attempting to load initial domain-config from " + domainConfigFile.getParentFile().getPath());
            InputStream domainConfigInputStream = new FileInputStream(domainConfigFile);
            domainConfig.load(domainConfigInputStream);
            parseDomainConfig(domain, domainConfig);
        } catch (FileNotFoundException fnfe) {
            LOG.fatal(DOMAIN_CONFIG_ERROR, fnfe);
        } catch (IOException ioe) {
            LOG.fatal(DOMAIN_CONFIG_ERROR, ioe);
        }
        return domain;
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
        getDomain();
    }
}
