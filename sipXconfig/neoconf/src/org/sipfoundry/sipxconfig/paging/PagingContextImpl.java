/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.PagingRule;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.dao.support.DataAccessUtils;

public class PagingContextImpl extends SipxHibernateDaoSupport implements PagingContext,
        BeanFactoryAware {

    private IntercomManager m_intercomManager;

    private BeanFactory m_beanFactory;

    public void setIntercomManager(IntercomManager intercomManager) {
        m_intercomManager = intercomManager;
    }

    private PagingServer getPagingServer() {
        List pagingServers = getHibernateTemplate().loadAll(PagingServer.class);
        PagingServer ps = (PagingServer) DataAccessUtils.singleResult(pagingServers);
        if (ps == null) {
            ps = (PagingServer) m_beanFactory.getBean(PagingServer.BEAN_NAME, PagingServer.class);
            getHibernateTemplate().save(ps);
        }
        return ps;
    }

    public String getPagingPrefix() {
        return getPagingServer().getPrefix();
    }

    public List<PagingGroup> getPagingGroups() {
        return getHibernateTemplate().loadAll(PagingGroup.class);
    }

    public PagingGroup getPagingGroupById(Integer pagingGroupId) {
        return (PagingGroup) getHibernateTemplate().load(PagingGroup.class, pagingGroupId);
    }

    public void savePagingPrefix(String prefix) {
        getPagingServer().setPrefix(prefix);
    }

    public void savePagingGroup(PagingGroup group) {
        getHibernateTemplate().saveOrUpdate(group);
    }

    public void deletePagingGroupsById(Collection<Integer> groupsIds) {
        removeAll(PagingGroup.class, groupsIds);
    }

    public void clear() {
        removeAll(PagingGroup.class);
    }

    public List< ? extends DialingRule> getDialingRules() {
        String prefix = getPagingPrefix();
        if (StringUtils.isEmpty(prefix)) {
            return Collections.emptyList();
        }
        String alertInfo = m_intercomManager.getIntercom().getCode();
        if (StringUtils.isEmpty(alertInfo)) {
            alertInfo = "alert";
        }
        PagingRule rule = new PagingRule(prefix, alertInfo);
        return Arrays.asList(rule);
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            List<PagingGroup> groups = getPagingGroups();
            for (PagingGroup group : groups) {
                Set<User> users = group.getUsers();
                if (users.remove(user)) {
                    getHibernateTemplate().saveOrUpdate(group);
                }
            }
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
