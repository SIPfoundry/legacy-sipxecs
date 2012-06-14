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
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.rls.RlsRule;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport implements SpeedDialManager {
    private CoreContext m_coreContext;

    @Override
    public SpeedDial getSpeedDialForUserId(Integer userId, boolean create) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(userId);
        if (!speeddials.isEmpty()) {
            return speeddials.get(0);
        }

        User user = m_coreContext.loadUser(userId);
        return getGroupSpeedDialForUser(user, create);
    }

    @Override
    public SpeedDial getSpeedDialForUser(User user, boolean create) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(user.getId());
        if (!speeddials.isEmpty()) {
            return speeddials.get(0);
        }

        return getGroupSpeedDialForUser(user, create);
    }

    public SpeedDial getGroupSpeedDialForUser(User user, boolean create) {
        Set<Group> groups = user.getGroups();
        if (groups.isEmpty() && !create) {
            return null;
        }
        List<SpeedDialGroup> speeddialGroups = new ArrayList<SpeedDialGroup>();
        for (Group group : groups) {
            speeddialGroups.addAll(findSpeedDialForGroupId(group.getId()));
        }
        if (!speeddialGroups.isEmpty()) {
            /*
             * If there are more than 1 group, choose the last group on the list that has a
             * non-zero number of speeddial buttons defined.
             */
            for (int i = speeddialGroups.size() - 1; i >= 0; i--) {
                if (0 < speeddialGroups.get(i).getButtons().size()) {
                    return speeddialGroups.get(i).getSpeedDial(user);
                }
            }
        }

        if (!create) {
            return null;
        }
        SpeedDial speedDial = new SpeedDial();
        speedDial.setUser(m_coreContext.loadUser(user.getId()));
        return speedDial;
    }

    @Override
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

    @Override
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

    @Override
    public void saveSpeedDial(SpeedDial speedDial) {
        getHibernateTemplate().saveOrUpdate(speedDial);
        getDaoEventPublisher().publishSave(speedDial.getUser());
    }

    /**
     * This method starts with "save" because we want to trigger speed dial replication (see ReplicationTrigger.java)
     */
    @Override
    public void speedDialSynchToGroup(SpeedDial speedDial) {
        deleteSpeedDialsForUser(speedDial.getUser().getId());
        getDaoEventPublisher().publishSave(speedDial.getUser());
    }

    @Override
    public void saveSpeedDialGroup(SpeedDialGroup speedDialGroup) {
        getHibernateTemplate().saveOrUpdate(speedDialGroup);
        getDaoEventPublisher().publishSave(speedDialGroup.getUserGroup());
    }

    @Override
    public void deleteSpeedDialsForGroup(int groupId) {
        List<SpeedDialGroup> groups = findSpeedDialForGroupId(groupId);
        getDaoEventPublisher().publishDeleteCollection(groups);
        getHibernateTemplate().deleteAll(groups);
    }

    @Override
    public void deleteSpeedDialsForUser(int userId) {
        List<SpeedDial> speedDials = findSpeedDialForUserId(userId);
        if (!speedDials.isEmpty()) {
            getDaoEventPublisher().publishDeleteCollection(speedDials);
            getHibernateTemplate().deleteAll(speedDials);
            getDaoEventPublisher().publishSave(m_coreContext.getUser(userId));
        }
    }

    @Override
    public List<DialingRule> getDialingRules(Location location) {
        DialingRule[] rules = new DialingRule[] {
            new RlsRule()
        };
        return Arrays.asList(rules);
    }


    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public void clear() {
        removeAll(SpeedDial.class);
    }
}
