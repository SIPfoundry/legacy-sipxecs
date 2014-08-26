/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sbc;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class SbcManagerImpl extends SipxHibernateDaoSupport implements SbcManager, BeanFactoryAware {
    private DomainManager m_domainManager;
    private BeanFactory m_beanFactory;

    public DefaultSbc getDefaultSbc() {
        List<DefaultSbc> sbcs = getHibernateTemplate().loadAll(DefaultSbc.class);
        DefaultSbc sbc = (DefaultSbc) DataAccessUtils.singleResult(sbcs);
        return sbc;
    }

    public DefaultSbc loadDefaultSbc() {
        DefaultSbc sbc = getDefaultSbc();
        if (sbc == null) {
            sbc = new DefaultSbc();
            sbc.setRoutes(createDefaultSbcRoutes());
            getHibernateTemplate().save(sbc);
            //Need to flush - since there can be only one Default SBC in the database.
            //Otherwise, the hibernate session may not be aware of the fact that a default SBC is
            //already saved, so  you may end up having two default SBCs in the database.
            getHibernateTemplate().flush();
            getDaoEventPublisher().publishSave(sbc);
        }
        return sbc;
    }

    public List<AuxSbc> loadAuxSbcs() {
        return getHibernateTemplate().loadAll(AuxSbc.class);
    }

    public void saveSbc(Sbc sbc) {
        if (sbc.isNew()) {
            getHibernateTemplate().save(sbc);
        } else {
            getHibernateTemplate().merge(sbc);
        }
    }

    public AuxSbc loadSbc(Integer sbcId) {
        return (AuxSbc) getHibernateTemplate().load(AuxSbc.class, sbcId);
    }

    public void removeSbcs(Collection<Integer> selectedRows) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection<AuxSbc> sbcs = DaoUtils.loadBeanByIds(hibernate, AuxSbc.class, selectedRows);
        getDaoEventPublisher().publishDeleteCollection(sbcs);
        hibernate.deleteAll(sbcs);
    }

    public void deleteSbc(Sbc sbc) {
        getHibernateTemplate().delete(sbc);
    }

    public SbcRoutes getRoutes() {
        SbcRoutes routes = new SbcRoutes();
        Set<String> sbcDomains = new HashSet<String>();
        Set<String> sbcSubnets = new HashSet<String>();
        List<Sbc> sbcs = getHibernateTemplate().loadAll(Sbc.class);
        for (Sbc sbc : sbcs) {
            sbcDomains.addAll(sbc.getRoutes().getDomains());
            sbcSubnets.addAll(sbc.getRoutes().getSubnets());
        }

        List<String> domains = new ArrayList<String>(sbcDomains);
        List<String> subnets = new ArrayList<String>(sbcSubnets);
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        return routes;
    }

    public void clear() {
        removeAll(Sbc.class);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    protected SbcRoutes createDefaultSbcRoutes() {
        Domain domain = m_domainManager.getDomain();
        String wildcard = String.format("*.%s", domain.getName());
        List<String> domains = new ArrayList<String>();
        domains.add(wildcard);

        SbcRoutes sbcRoutes = (SbcRoutes) m_beanFactory.getBean("defaultSbcRoutes",
                SbcRoutes.class);
        sbcRoutes.setDomains(domains);

        return sbcRoutes;
    }
}
