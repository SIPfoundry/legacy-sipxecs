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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Process;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.PagingRule;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;

public class PagingContextImpl extends SipxHibernateDaoSupport implements PagingContext {

    private PagingConfiguration m_pagingConfiguration;

    private SipxReplicationContext m_replicationContext;

    private IntercomManager m_intercomManager;

    private SipxProcessContext m_processContext;

    public void setPagingConfiguration(PagingConfiguration pagingConfiguration) {
        m_pagingConfiguration = pagingConfiguration;
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public void setIntercomManager(IntercomManager intercomManager) {
        m_intercomManager = intercomManager;
    }

    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }

    public String getPagingPrefix() {
        List pagingGroups = getHibernateTemplate().loadAll(PagingGroup.class);
        String prefix = "";
        if (pagingGroups.size() > 0) {
            PagingGroup pagingGroup = (PagingGroup) pagingGroups.get(0);
            prefix = pagingGroup.getPrefix();
        }
        return prefix;
    }

    public List<PagingGroup> getPagingGroups() {
        return getHibernateTemplate().loadAll(PagingGroup.class);
    }

    public PagingGroup getPagingGroupById(Integer pagingGroupId) {
        return (PagingGroup) getHibernateTemplate().load(PagingGroup.class, pagingGroupId);
    }

    public void savePagingPrefix(String prefix) {
        for (PagingGroup group : getPagingGroups()) {
            group.setPrefix(prefix);
            getHibernateTemplate().saveOrUpdate(group);
        }
    }

    public void savePagingGroup(PagingGroup group) {
        if (group.getPrefix() == null) {
            group.setPrefix(getPagingPrefix());
        }
        getHibernateTemplate().saveOrUpdate(group);
        // flush to take effect in order to generate the page group config
        getHibernateTemplate().flush();
        replicatePagingConfig();
    }

    public void deletePagingGroupsById(Collection<Integer> groupsIds) {
        if (groupsIds.isEmpty()) {
            // no groups to delete => nothing to do
            return;
        }
        List<PagingGroup> groups = new ArrayList<PagingGroup>();
        for (Integer groupId : groupsIds) {
            groups.add(getPagingGroupById(groupId));
        }
        getHibernateTemplate().deleteAll(groups);
        // flush to take effect in order to generate the page group config
        getHibernateTemplate().flush();
        replicatePagingConfig();
    }

    public void clear() {
        Collection c = getHibernateTemplate().loadAll(PagingGroup.class);
        getHibernateTemplate().deleteAll(c);
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

    public void replicatePagingConfig() {
        replicatePagingConfig(m_replicationContext);
    }

    /**
     * Allows the caller to specify a replication context to use
     */
    public void replicatePagingConfig(SipxReplicationContext context) {
        m_pagingConfiguration.generate(getPagingGroups());
        context.replicate(m_pagingConfiguration);
    }

    public void restartService() {
        Process service = m_processContext.getProcess(ProcessName.PAGE_SERVER);
        Collection<Process> services = new ArrayList<Process>();
        services.add(service);
        m_processContext.manageServices(services, SipxProcessContext.Command.RESTART);
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
            replicatePagingConfig();
        }
    }
}
