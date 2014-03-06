/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alias;

import java.lang.reflect.Proxy;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.BeanInitializationException;
import org.springframework.beans.factory.ListableBeanFactory;


// DO_NEXT: Move extension-related classes into this package and move any bean definitions into
// this beans.xml.

/**
 * AliasManagerImpl: manages all SIP aliases
 */
public class AliasManagerImpl extends SipxHibernateDaoSupport implements AliasManager, BeanFactoryAware {
    public static final String CONTEXT_BEAN_NAME = "aliasManagerImpl";
    private static final Log LOG = LogFactory.getLog(AliasManagerImpl.class);
    private Collection<AliasOwner> m_aliasOwners;
    private ListableBeanFactory m_beanFactory;

    /** Return true if the alias is in use by some AliasOwner */
    public boolean isAliasInUse(String alias) {
        for (AliasOwner owner : getAliasOwners()) {
            if (owner.isAliasInUse(alias)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Return the bean IDs of objects with this alias by asking all the AliasOwners. Return null
     * if there is no such object.
     */
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection objects = new ArrayList();
        for (AliasOwner owner : getAliasOwners()) {
            Collection ids = owner.getBeanIdsOfObjectsWithAlias(alias);
            if (ids != null) {
                objects.addAll(ids);
            }
        }
        return objects;
    }

    public boolean canObjectUseAlias(BeanWithId bean, String alias) {
        if (alias == null) {
            return true;
        }
        if (bean.isNew()) {
            // For a new bean, if there is any database object that has already claimed
            // the alias, then this bean cannot use it
            boolean canUseAlias = !isAliasInUse(alias);
            if (LOG.isInfoEnabled() && !canUseAlias) {
                String msg = MessageFormat.format("NEW bean; Alias \"{0}\" is in use by an existing bean",
                    new Object[] {alias});
                LOG.info(msg);
            }
            return canUseAlias;
        }

        boolean canUseAlias = false;
        Collection bids = getBeanIdsOfObjectsWithAlias(alias);
        int size = bids.size();
        if (size == 0) {
            // No one is using the alias so we can take it
            canUseAlias = true;
        } else if (size == 1) {
            // One object is using the alias. If that object is the bean itself, then
            // it's OK for the bean to use the alias, otherwise not.
            BeanId bid = (BeanId) bids.iterator().next();
            canUseAlias = bid.isIdOfBean(bean);
            if (LOG.isInfoEnabled() && !canUseAlias) {
                String msg = MessageFormat.format("Existing bean; Alias \"{0}\" is in use by: {1}",
                    new Object[] {alias, bid.toString()});
                LOG.info(msg);
            }
        } else {
            // More than one object is using the alias! If the bean is one of them,
            // then the bean can continue to use the alias, but log an error in any
            // case because the database is messed up.
            for (Iterator iter = bids.iterator(); iter.hasNext();) {
                BeanId bid = (BeanId) iter.next();
                if (bid.isIdOfBean(bean)) {
                    canUseAlias = true;
                    break;
                }
            }
            if (LOG.isErrorEnabled()) {
                String msg = MessageFormat.format("Alias \"{0}\" is in use multiple objects:",
                    new Object[] {alias});
                LOG.error(msg + StringUtils.join(bids.iterator(), ' '));
            }
        }
        return canUseAlias;
    }

    /** Return all alias owners (excluding the AliasManagerImpl itself) */
    protected Collection<AliasOwner> getAliasOwners() {
        if (m_aliasOwners == null) { // lazy initialization
            if (m_beanFactory == null) {
                throw new BeanInitializationException(getClass().getName() + " not initialized");
            }
            Map<String, AliasOwner> beanMap = m_beanFactory.getBeansOfType(AliasOwner.class, false, true);
            m_aliasOwners = new ArrayList<AliasOwner>(beanMap.size());
            // Collect all proxies
            for (AliasOwner owner : beanMap.values()) {
                // Only include beans created through Factories - need Hibernate support.
                // Exclude the AliasManagerImpl itself, or we'll get into infinite recursion
                // when calling the alias owners.
                if (owner instanceof Proxy && !(owner instanceof AliasManager)) {
                    m_aliasOwners.add(owner);
                }
            }

        }
        return m_aliasOwners;
    }

    public String getNextAvailableNumericBasedAlias(String desiredAlias) {
        String alias = desiredAlias;
        if (isAliasInUse(alias)) {
            // TODO : attempt to find an available alias
            throw new UserException("alias in use");
        }
        return alias;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
        m_aliasOwners = null;
    }

}
