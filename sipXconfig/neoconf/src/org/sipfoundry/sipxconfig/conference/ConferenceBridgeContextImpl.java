/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeContextImpl extends HibernateDaoSupport implements BeanFactoryAware,
        ConferenceBridgeContext, AliasProvider {
    private static final String CONFERENCE = "conference";
    private static final String VALUE = "value";
    private static final String CONFERENCE_IDS_WITH_ALIAS = "conferenceIdsWithAlias";

    private AliasManager m_aliasManager;
    private BeanFactory m_beanFactory;
    private ConferenceBridgeProvisioning m_provisioning;
    private CoreContext m_coreContext;

    public List getBridges() {
        return getHibernateTemplate().loadAll(Bridge.class);
    }

    public void store(Bridge bridge) {
        getHibernateTemplate().saveOrUpdate(bridge);
        if (bridge.isNew()) {
            // need to make sure that ID is set
            getHibernateTemplate().flush();
        }
        m_provisioning.deploy(bridge.getId());
    }

    public void store(Conference conference) {
        validate(conference);
        getHibernateTemplate().saveOrUpdate(conference);
        m_provisioning.deploy(conference.getBridge().getId());
    }

    public void validate(Conference conference) {
        String name = conference.getName();
        String extension = conference.getExtension();
        if (name == null) {
            throw new UserException("A conference must have a name");
        }
        if (extension == null) {
            throw new UserException("A conference must have an extension");
        }

        final String conferenceTypeName = CONFERENCE;
        if (!m_aliasManager.canObjectUseAlias(conference, name)) {
            throw new NameInUseException(conferenceTypeName, name);
        }
        if (!m_aliasManager.canObjectUseAlias(conference, extension)) {
            throw new ExtensionInUseException(conferenceTypeName, extension);
        }
    }

    public Bridge newBridge() {
        return (Bridge) m_beanFactory.getBean(Bridge.BEAN_NAME, Bridge.class);
    }

    public Conference newConference() {
        Conference conference = (Conference) m_beanFactory.getBean(Conference.BEAN_NAME,
                Conference.class);
        conference.generateAccessCodes();
        return conference;
    }

    public void removeBridges(Collection bridgesIds) {
        List bridges = new ArrayList(bridgesIds.size());
        for (Iterator i = bridgesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            Bridge bridge = loadBridge(id);
            bridges.add(bridge);
        }
        getHibernateTemplate().deleteAll(bridges);
    }

    public void removeConferences(Collection conferencesIds) {
        Set bridges = new HashSet();
        for (Iterator i = conferencesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            Conference conference = loadConference(id);
            Bridge bridge = conference.getBridge();
            bridge.removeConference(conference);
            bridges.add(bridge);
        }
        getHibernateTemplate().saveOrUpdateAll(bridges);
    }

    public Bridge loadBridge(Serializable id) {
        return (Bridge) getHibernateTemplate().load(Bridge.class, id);
    }

    public Conference loadConference(Serializable id) {
        return (Conference) getHibernateTemplate().load(Conference.class, id);
    }

    public void clear() {
        List bridges = getBridges();
        getHibernateTemplate().deleteAll(bridges);
    }

    // trivial get/set
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setProvisioning(ConferenceBridgeProvisioning provisioning) {
        m_provisioning = provisioning;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public boolean isAliasInUse(String alias) {
        List confIds = getHibernateTemplate().findByNamedQueryAndNamedParam(
                CONFERENCE_IDS_WITH_ALIAS, VALUE, alias);
        return SipxCollectionUtils.safeSize(confIds) > 0;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(
                CONFERENCE_IDS_WITH_ALIAS, VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, Conference.class);
        return bids;
    }

    public Collection getAliasMappings() {
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        final ArrayList list = new ArrayList();
        for (Iterator i = conferences.iterator(); i.hasNext();) {
            Conference conference = (Conference) i.next();
            list.addAll(conference.generateAliases(m_coreContext.getDomainName()));
        }
        return list;
    }
}
