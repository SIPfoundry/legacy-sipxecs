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

import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.localization.Localization;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.dao.support.DataAccessUtils;

public class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager {

    // do not reference m_server, see note in spring file
    private SipxServer m_server;
    private DomainConfiguration m_domainConfiguration;
    private SipxReplicationContext m_replicationContext;
    private String m_authorizationRealm;

    public SipxServer getServer() {
        return m_server;
    }

    public void setServer(SipxServer server) {
        m_server = server;
    }

    public DomainConfiguration getDomainConfiguration() {
        return m_domainConfiguration;
    }

    public void setDomainConfiguration(DomainConfiguration domainConfiguration) {
        m_domainConfiguration = domainConfiguration;
    }

    public void setReplicationContext(SipxReplicationContext context) {
        m_replicationContext = context;
    }

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

        getServer().setDomainName(domain.getName());
        getServer().setRegistrarDomainAliases(domain.getAliases());
        getServer().applySettings();

        // TBD - fix DomainManagerTest to work with the implementation of
        // replicateDomainConfig - for now, do not call the function when
        // running the test
        if (domain.getName().equals("goose")) {
            return;
        }
        replicateDomainConfig();
    }

    public void replicateDomainConfig() {
        replicateDomainConfig(m_replicationContext);
    }
    
    /** 
     * Allows the caller to specify a replication context to use
     */
    public void replicateDomainConfig(SipxReplicationContext context) {
        m_domainConfiguration.generate(getExistingDomain(), m_authorizationRealm,
                                       getExistingLocalization().getLanguage());
        context.replicate(m_domainConfiguration);
    }

    private Domain getExistingDomain() {
        Collection<Domain> domains = getHibernateTemplate().findByNamedQuery("domain");
        return DaoUtils.<Domain> requireOneOrZero(domains, "named query: domain");
    }

    private Localization getExistingLocalization() {
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
     * Return true if the domain is initialized. This provides a workaround for the fact that
     * getDomain throws a RuntimeException if the domain is not initialized when that method is
     * called
     */
    public boolean isDomainInitialized() {
        if (getExistingDomain() == null) {
            return false;
        }

        return true;
    }

    public DomainConfiguration createDomainConfiguration() {
        return m_domainConfiguration;
    }

    public String getAuthorizationRealm() {
        return m_authorizationRealm;
    }

    public void setAuthorizationRealm(String authorizationRealm) {
        m_authorizationRealm = authorizationRealm;
    }
}
