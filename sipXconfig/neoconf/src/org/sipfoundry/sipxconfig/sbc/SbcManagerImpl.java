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
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.event.DaoEventPublisher;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SbcManagerImpl extends HibernateDaoSupport implements SbcManager, BeanFactoryAware {
    private DomainManager m_domainManager;
    private BeanFactory m_beanFactory;
    private DaoEventPublisher m_daoEventPublisher;

    public DefaultSbc loadDefaultSbc() {
        List sbcs = getHibernateTemplate().loadAll(DefaultSbc.class);
        DefaultSbc sbc = (DefaultSbc) DataAccessUtils.singleResult(sbcs);
        if (sbc == null) {
            sbc = new DefaultSbc();
            sbc.setRoutes(createDefaultSbcRoutes());
            getHibernateTemplate().save(sbc);
            //Need to flush - since there can be only one Default SBC in the database.
            //Otherwise, the hibernate session may not be aware of the fact that a default SBC is
            //already saved, so  you may end up having two default SBCs in the database.
            getHibernateTemplate().flush();
            m_daoEventPublisher.publishSave(sbc);
        }
        return sbc;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures() {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(Location l) {
        return Collections.singleton(FEATURE);
    }

    public List<AuxSbc> loadAuxSbcs() {
        return getHibernateTemplate().loadAll(AuxSbc.class);
    }

    public void saveSbc(Sbc sbc) {
        getHibernateTemplate().saveOrUpdate(sbc);
    }

    public AuxSbc loadSbc(Integer sbcId) {
        return (AuxSbc) getHibernateTemplate().load(AuxSbc.class, sbcId);
    }

    public void removeSbcs(Collection<Integer> selectedRows) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection<AuxSbc> sbcs = DaoUtils.loadBeanByIds(hibernate, AuxSbc.class, selectedRows);
        hibernate.deleteAll(sbcs);
        for (AuxSbc sbc : sbcs) {
            m_daoEventPublisher.publishDelete(sbc);
        }
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
            routes = sbc.getRoutes();
            sbcDomains.addAll(routes.getDomains());
            sbcSubnets.addAll(routes.getSubnets());
        }

        List<String> domains = new ArrayList<String>(sbcDomains);
        List<String> subnets = new ArrayList<String>(sbcSubnets);
        routes.setDomains(domains);
        routes.setSubnets(subnets);

        return routes;
    }

    public void clear() {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection<Sbc> sbcs = hibernate.loadAll(Sbc.class);
        hibernate.deleteAll(sbcs);
        for (Sbc sbc : sbcs) {
            m_daoEventPublisher.publishDelete(sbc);
        }
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
