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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.localization.Localization;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.dao.support.DataAccessUtils;

public abstract class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager {

    private String m_authorizationRealm;
    private String m_alarmServerUrl;
    private String m_initialDomain;
    private String m_initialAlias;
    private String m_configServerHost;

    protected abstract DomainConfiguration createDomainConfiguration();

    protected abstract SipxReplicationContext getReplicationContext();

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

        replicateDomainConfig();
    }

    public void replicateDomainConfig() {
        Domain existingDomain = getExistingDomain();
        if (existingDomain == null) {
            throw new DomainNotInitializedException();
        }
        DomainConfiguration domainConfiguration = createDomainConfiguration();
        domainConfiguration.generate(existingDomain, m_authorizationRealm, m_configServerHost,
                getExistingLocalization().getLanguage(), m_alarmServerUrl);
        getReplicationContext().replicate(domainConfiguration);
        getReplicationContext().publishEvent(new DomainConfigReplicatedEvent(this));
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
     * Initialize the firs and single domain supported by sipX at the moment. If domain does not
     * exist a new domain will be created and initialized with a 'm_initialDomain' name. If domain
     * does exist we make sure it has secret initialed - new secret is created if the secret is
     * empty.
     *
     * It is save to call this function many times. Only the first call should result in actually
     * saving and replicating the domain
     */
    public void initialize() {
        Domain domain = getExistingDomain();
        if (domain == null) {
            domain = new Domain();
            domain.setName(m_initialDomain);
            if (StringUtils.isNotBlank(m_initialAlias)) {
                domain.addAlias(m_initialAlias);
            }
        }
        if (domain.initSecret()) {
            saveDomain(domain);
        }
    }

    public String getAuthorizationRealm() {
        return m_authorizationRealm;
    }

    public void setAuthorizationRealm(String authorizationRealm) {
        m_authorizationRealm = authorizationRealm;
    }

    public String getAlarmServerUrl() {
        return m_alarmServerUrl;
    }

    public void setAlarmServerUrl(String alarmServerUrl) {
        m_alarmServerUrl = alarmServerUrl;
    }

    public void setInitialDomain(String initialDomain) {
        m_initialDomain = initialDomain;
    }

    public void setInitialAlias(String initialAlias) {
        m_initialAlias = initialAlias;
    }

    public void setConfigServerHost(String configServerHost) {
        m_configServerHost = configServerHost;
    }
}
