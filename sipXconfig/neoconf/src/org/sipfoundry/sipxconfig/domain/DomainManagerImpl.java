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


import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.localization.Localization;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.JdbcTemplate;

public class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager, SetupListener {
    private static DomainManagerImpl s_instance;
    private JdbcTemplate m_jdbc;
    private volatile Domain m_domain;
    private String m_configuredDomain;
    private String m_configuredSipDomain;
    private String m_configuredRealm;
    private String m_configuredSecret;
    private String m_configuredFqdn;
    private String m_configuredIp;

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
                        Domain config = new Domain();
                        config.setName(getEffectiveSipDomain());
                        config.setSharedSecret(m_configuredSecret);
                        config.setSipRealm(m_configuredRealm);
                        config.addAlias(m_configuredIp);
                        config.addAlias(m_configuredFqdn);
                        getHibernateTemplate().saveOrUpdate(config);
                        d2 = reloadDomainFromDb();
                    }
                    d2.setNetworkName(m_configuredDomain);

                    m_domain = d2;
                }
            }
        }

        return m_domain;
    }

    void changeDomainName(String domain) {
        // Ran into problems exec-ing a stored proc, this gave error about
        // response when none was expected
        //    m_jdbc.update("select change_domain_on_restore(?)", domain);
        //
        m_jdbc.execute("select change_domain_on_restore('" + domain + "')");
        m_domain = null;
    }

    private Domain reloadDomainFromDb() {
        getHibernateTemplate().flush();
        Domain reload = loadDomainFromDb();
        if (reload == null) {
            throw new DomainNotInitializedException();
        }
        return reload;
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

    @Override
    public boolean setup(SetupManager manager) {
        Domain d = getDomain();
        if (!d.getName().equals(getEffectiveSipDomain())) {
            changeDomainName(getEffectiveSipDomain());
        }

        return true;
    }

    public void setConfiguredDomain(String configuredDomain) {
        m_configuredDomain = configuredDomain;
    }

    public void setConfiguredRealm(String configuredRealm) {
        m_configuredRealm = configuredRealm;
    }

    public void setConfiguredSecret(String configuredSecret) {
        m_configuredSecret = configuredSecret;
    }

    public void setConfiguredIp(String configuredIp) {
        m_configuredIp = configuredIp;
    }

    public void setConfiguredFqdn(String configuredFqdn) {
        m_configuredFqdn = configuredFqdn;
    }

    public void setJdbc(JdbcTemplate jdbc) {
        m_jdbc = jdbc;
    }

    private String getEffectiveSipDomain() {
        return m_configuredSipDomain == null ? m_configuredDomain : m_configuredSipDomain;
    }

    public void setConfiguredSipDomain(String configuredSipDomain) {
        m_configuredSipDomain = configuredSipDomain;
    }

    public void setTestDomain(Domain d) {
        m_domain = d;
    }
}
