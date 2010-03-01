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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanActivationManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.PagingRule;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

public abstract class PagingContextImpl extends SipxHibernateDaoSupport implements PagingContext {
    /** Default ALERT-INFO - hardcoded in Polycom phone configuration */
    private static final String ALERT_INFO = "sipXpage";

    private static final String PARAM_PAGING_GROUP_NUMBER = "pageGroupNumber";

    private static final String PARAM_PAGING_GROUP_ID = "pagingGroupId";

    private DialPlanActivationManager m_dialPlanActivationManager;

    private PagingProvisioningContext m_pagingProvisioningContext;

    protected abstract PagingServer createPagingServer();

    public PagingServer getPagingServer() {
        List pagingServers = getHibernateTemplate().loadAll(PagingServer.class);
        PagingServer ps = (PagingServer) DataAccessUtils.singleResult(pagingServers);
        if (ps == null) {
            ps = createPagingServer();
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

    public void setPagingPrefix(String prefix) {
        getPagingServer().setPrefix(prefix);
        m_dialPlanActivationManager.replicateDialPlan(true);
    }

    public String getSipTraceLevel() {
        return getPagingServer().getSipTraceLevel();
    }

    public void setSipTraceLevel(String traceLevel) {
        getPagingServer().setSipTraceLevel(traceLevel);
    }

    public void savePagingGroup(PagingGroup group) {
        if (group.isNew()) {
            // check if new object
            checkForDuplicateNames(group);
        } else {
            // on edit action - check if the group number for this group was modified
            // if the group number was changed then perform duplicate group number checking
            if (isNameChanged(group)) {
                checkForDuplicateNames(group);
            }
        }

        getHibernateTemplate().saveOrUpdate(group);
    }

    private void checkForDuplicateNames(PagingGroup group) {
        if (isNameInUse(group)) {
            throw new UserException("error.duplicateGroupNumbers");
        }
    }

    private boolean isNameInUse(PagingGroup group) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "anotherPagingGroupWithSameName", new String[] {
                    PARAM_PAGING_GROUP_NUMBER
                }, new Object[] {
                    group.getPageGroupNumber()
                });

        return DataAccessUtils.intResult(count) > 0;
    }

    private boolean isNameChanged(PagingGroup group) {
        List count = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "countPagingGroupWithSameName", new String[] {
                    PARAM_PAGING_GROUP_ID, PARAM_PAGING_GROUP_NUMBER
                }, new Object[] {
                    group.getId(), group.getPageGroupNumber()
                });

        return DataAccessUtils.intResult(count) == 0;
    }

    public void deletePagingGroupsById(Collection<Integer> groupsIds) {
        removeAll(PagingGroup.class, groupsIds);
        m_pagingProvisioningContext.deploy();
    }

    public void clear() {
        removeAll(PagingGroup.class);
    }

    public List< ? extends DialingRule> getDialingRules() {
        String prefix = getPagingPrefix();
        if (StringUtils.isEmpty(prefix)) {
            return Collections.emptyList();
        }
        PagingRule rule = new PagingRule(prefix, ALERT_INFO);
        return Arrays.asList(rule);
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    @Required
    public void setDialPlanActivationManager(DialPlanActivationManager dialPlanActivationManager) {
        m_dialPlanActivationManager = dialPlanActivationManager;
    }

    public void setPagingProvisioningContext(PagingProvisioningContext pagingProvisioningContext) {
        m_pagingProvisioningContext = pagingProvisioningContext;
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            boolean affectPaging = false;
            List<PagingGroup> groups = getPagingGroups();
            for (PagingGroup group : groups) {
                Set<User> users = group.getUsers();
                if (users.remove(user)) {
                    getHibernateTemplate().saveOrUpdate(group);
                    getHibernateTemplate().flush();
                    affectPaging = true;
                }
            }
            if (affectPaging) {
                m_pagingProvisioningContext.deploy();
            }
        }
    }
}
