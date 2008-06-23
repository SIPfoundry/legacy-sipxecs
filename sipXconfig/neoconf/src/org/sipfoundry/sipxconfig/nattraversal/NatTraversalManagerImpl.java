/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

import static org.springframework.dao.support.DataAccessUtils.singleResult;

public class NatTraversalManagerImpl extends SipxHibernateDaoSupport<NatTraversal> implements NatTraversalManager,
        BeanFactoryAware {

    private BeanFactory m_beanFactory;

    public void store(NatTraversal natTraversal) {
        saveBeanWithSettings(natTraversal);
    }

    public NatTraversal getNatTraversal() {
        List nats = getHibernateTemplate().loadAll(NatTraversal.class);
        NatTraversal natTraversal = (NatTraversal) singleResult(nats);
        return natTraversal;
    }

    /**
     * Save in database the default settings for NAT Traversal once the application is initialized -
     * since replication process uses READ ONLY hibernate session. NAT Traversal has to be saved
     * before replicaion This method is called only once (after application initialization) in
     * NatTarversalInit listener
     */
    public void saveDefaultNatTraversal() {
        List nats = getHibernateTemplate().loadAll(NatTraversal.class);
        NatTraversal natTraversal = (NatTraversal) singleResult(nats);
        // create a new one if one doesn't exists, otherwise
        // risk having 2 or more in database
        if (natTraversal == null) {
            natTraversal = (NatTraversal) m_beanFactory.getBean("natTraversal");
            store(natTraversal);
            // make sure that hibernate session is synchronized with database data
            getHibernateTemplate().flush();
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
