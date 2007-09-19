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

import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.event.EntitySaveListener;

public class DomainManagerImpl extends SipxHibernateDaoSupport<Domain> implements DomainManager {
    
    // do not reference m_server, see note in spring file
    private SipxServer m_server;
    private DomainConfigGenerator m_domainConfigGenerator;

    public SipxServer getServer() {
        return m_server;
    }

    public void setServer(SipxServer server) {
        m_server = server;
    }

    public DomainConfigGenerator getDomainConfigGenerator() {
        return m_domainConfigGenerator;
    }
    
    public void setDomainConfigGenerator(DomainConfigGenerator domainConfigGenerator) {
        m_domainConfigGenerator = domainConfigGenerator;
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
    }
    
    private Domain getExistingDomain() {
        Collection<Domain> domains = getHibernateTemplate().findByNamedQuery("domain");
        Domain domain = (Domain) DaoUtils.<Domain>requireOneOrZero(domains, "named query: domain");
        return domain;        
    }

    public List<DialingRule> getDialingRules() {
        List<DialingRule> rules;
        Domain d = getDomain();
        if (d.hasAliases()) {
            DialingRule domainRule = new DomainDialingRule(getDomain());
            rules = Collections.<DialingRule>singletonList(domainRule);
        } else {
            rules = Collections.<DialingRule>emptyList();
        }
        return rules;
    }  
    
    public EntitySaveListener<Domain> createDomainSaveListener() {
        return new DomainSaveListener();
    }
    
    private class DomainSaveListener extends EntitySaveListener<Domain> {
        public DomainSaveListener() {
            super(Domain.class);
        }
        
        protected void onEntitySave(Domain domain) {
            getDomainConfigGenerator().generate(getDomain());
        }
    }
}
