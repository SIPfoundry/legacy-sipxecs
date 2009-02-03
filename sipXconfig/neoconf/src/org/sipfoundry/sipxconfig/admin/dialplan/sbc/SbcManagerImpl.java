/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SbcManagerImpl extends HibernateDaoSupport implements SbcManager, BeanFactoryAware {
    private DomainManager m_domainManager;

    private BeanFactory m_beanFactory;

    private DialPlanActivationManager m_dialPlanActivationManager;

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
        }
        return sbc;
    }

    public List<AuxSbc> loadAuxSbcs() {
        return getHibernateTemplate().loadAll(AuxSbc.class);
    }

    public void saveSbc(Sbc sbc) {
        getHibernateTemplate().saveOrUpdate(sbc);
        m_dialPlanActivationManager.replicateDialPlan(true);
    }

    public AuxSbc loadSbc(Integer sbcId) {
        return (AuxSbc) getHibernateTemplate().load(AuxSbc.class, sbcId);
    }

    public void removeSbcs(Collection<Integer> selectedRows) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection sbcs = DaoUtils.loadBeanByIds(hibernate, AuxSbc.class, selectedRows);
        hibernate.deleteAll(sbcs);
    }

    public void clear() {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection sbcs = hibernate.loadAll(Sbc.class);
        hibernate.deleteAll(sbcs);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
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
