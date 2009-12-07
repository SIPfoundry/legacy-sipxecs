/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.common.event.UserGroupSaveDeleteListener;
import org.sipfoundry.sipxconfig.service.ConfigFileActivationManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport implements SpeedDialManager {

    private CoreContext m_coreContext;

    private ConfigFileActivationManager m_configFileManager;

    public SpeedDial getSpeedDialForUserId(Integer userId, boolean create) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(userId);
        if (!speeddials.isEmpty()) {
            return speeddials.get(0);
        }

        User user = m_coreContext.loadUser(userId);
        Set<Group> groups = user.getGroups();
        List<SpeedDialGroup> speeddialGroups = new ArrayList<SpeedDialGroup>();
        for (Group group : groups) {
            speeddialGroups.addAll(findSpeedDialForGroupId(group.getId()));
        }
        if (!speeddialGroups.isEmpty()) {
            /*
             * If there are more than 1 group, choose the first group on the list that has a
             * non-zero number of speeddial buttons defined.
             */
            for (SpeedDialGroup speedDialGroup : speeddialGroups) {
                if (0 < speedDialGroup.getButtons().size()) {
                    return speedDialGroup.getSpeedDial(user);
                }
            }
        }

        if (!create) {
            return null;
        }
        SpeedDial speedDial = new SpeedDial();
        speedDial.setUser(m_coreContext.loadUser(userId));
        return speedDial;
    }

    public SpeedDialGroup getSpeedDialForGroupId(Integer groupId) {
        List<SpeedDialGroup> speedDialGroups = findSpeedDialForGroupId(groupId);
        if (!speedDialGroups.isEmpty()) {
            return speedDialGroups.get(0);
        }

        SpeedDialGroup speedDialGroup = new SpeedDialGroup();
        Group userGroup = m_coreContext.getGroupById(groupId);
        speedDialGroup.setUserGroup(userGroup);
        return speedDialGroup;
    }

    public boolean isSpeedDialDefinedForUserId(Integer userId) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(userId);
        if (!speeddials.isEmpty()) {
            return true;
        }
        return false;
    }

    private List<SpeedDial> findSpeedDialForUserId(Integer userId) {
        List<SpeedDial> speeddials = getHibernateTemplate().findByNamedQueryAndNamedParam("speedDialForUserId",
                "userId", userId);
        return speeddials;
    }

    private List<SpeedDialGroup> findSpeedDialForGroupId(Integer groupId) {
        List<SpeedDialGroup> speeddialGroups = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "speedDialForGroupId", "userGroupId", groupId);
        return speeddialGroups;
    }

    public void saveSpeedDial(SpeedDial speedDial) {
        getHibernateTemplate().saveOrUpdate(speedDial);
        activateResourceList();
    }

    public void saveSpeedDialGroup(SpeedDialGroup speedDialGroup) {
        getHibernateTemplate().saveOrUpdate(speedDialGroup);
        activateResourceList();
    }

    public void deleteSpeedDialsForUser(int userId) {
        List<SpeedDial> speedDials = findSpeedDialForUserId(userId);
        if (!speedDials.isEmpty()) {
            getHibernateTemplate().deleteAll(speedDials);
            activateResourceList();
        }
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    public UserGroupSaveDeleteListener createUserGroupDeleteListener() {
        return new OnUserGroupDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        @Override
        protected void onUserDelete(User user) {
            deleteSpeedDialsForUser(user.getId());
        }
    }

    private class OnUserGroupDelete extends UserGroupSaveDeleteListener {
        @Override
        protected void onUserGroupDelete(Group user) {
            List<SpeedDialGroup> speedDialGroups = findSpeedDialForGroupId(user.getId());
            if (!speedDialGroups.isEmpty()) {
                getHibernateTemplate().deleteAll(speedDialGroups);
                activateResourceList();
            }
        }
    }

    public List<DialingRule> getDialingRules() {
        DialingRule[] rules = new DialingRule[] {
            new RlsRule()
        };
        return Arrays.asList(rules);
    }

    public void activateResourceList() {
        m_configFileManager.activateConfigFiles();
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setRlsConfigFilesActivator(ConfigFileActivationManager configFileManager) {
        m_configFileManager = configFileManager;
    }

    public void clear() {
        Collection c = getHibernateTemplate().loadAll(SpeedDial.class);
        getHibernateTemplate().deleteAll(c);
    }
}
