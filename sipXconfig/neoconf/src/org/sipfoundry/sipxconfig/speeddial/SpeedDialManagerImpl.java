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
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.UserValidationUtils;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.common.event.DaoEventListenerAdvanced;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.rls.RlsRule;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport<SpeedDial> implements SpeedDialManager,
        DaoEventListener, DaoEventListenerAdvanced {
    private static final int MAX_BUTTONS = 136;
    private CoreContext m_coreContext;
    private FeatureManager m_featureManager;
    private ConfigManager m_configManager;
    private ValidUsers m_validUsers;
    private SipxReplicationContext m_sipxReplicationContext;

    private AliasManager m_aliasManager;
    private String m_featureId;

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

    @Override
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

    @Override
    public List<SpeedDial> findSpeedDialForUserId(Integer userId) {
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
        verifyBlfs(speedDial.getButtons());
        if (speedDial.isNew()) {
            getHibernateTemplate().save(speedDial);
        } else {
            getHibernateTemplate().merge(speedDial);
        }
        getHibernateTemplate().flush();
        User user = m_coreContext.loadUser(speedDial.getUser().getId());
        getDaoEventPublisher().publishSave(user);
    }

    private void verifyBlfs(List<Button> buttons) {
        int count = 0;
        for (Button button : buttons) {
            count++;
            if (button.isBlf()) {
                String number = button.getNumber();
                if (!UserValidationUtils.isValidEmail(number) && !m_aliasManager.isAliasInUse(number)) {
                    button.setBlf(false);
                    throw new UserException("&error.notValidBlf", number);
                }
            }
        }
        if (count > MAX_BUTTONS) {
            throw new UserException("&error.speedDialExceedsMaxNumber", MAX_BUTTONS);
        }
    }

    /**
     * This method starts with "save" because we want to trigger speed dial replication (see
     * ReplicationTrigger.java)
     */
    @Override
    public void speedDialSynchToGroup(User user) {
        deleteSpeedDialsForUser(user.getId());
        getHibernateTemplate().flush();
        getDaoEventPublisher().publishSave(user);
    }

    @Override
    public void saveSpeedDialGroup(SpeedDialGroup speedDialGroup) {
        verifyBlfs(speedDialGroup.getButtons());
        if (speedDialGroup.isNew()) {
            getHibernateTemplate().save(speedDialGroup);
        } else {
            getHibernateTemplate().merge(speedDialGroup);
        }
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
        }
    }

    @Override
    public List<DialingRule> getDialingRules(Location location) {
        if (!m_featureManager.isFeatureEnabled(new LocationFeature(m_featureId))) {
            return Collections.emptyList();
        }

        DialingRule[] rules = new DialingRule[] {
            new RlsRule()
        };
        return Arrays.asList(rules);
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public void clear() {
        removeAll(SpeedDial.class);

        // A little convoluted, but only way i could keep mongo and postgres in sync
        // which is critical for resource-lists.xml.
        m_validUsers.removeFieldFromUsers(MongoConstants.SPEEDDIAL);
        m_configManager.configureEverywhere(Rls.FEATURE);
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    public ValidUsers getValidUsers() {
        return m_validUsers;
    }

    @Required
    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    @Required
    public void setAliasManager(AliasManager aliasMgr) {
        m_aliasManager = aliasMgr;
    }

    @Required
    public void setFeatureId(String feature) {
        m_featureId = feature;
    }

    @Override
    public void onDelete(Object entity) {
        // TODO: on group deletion publish delete is called after the delete (unlike for users)
        replicateXmppSpecialUser(entity);
    }

    @Override
    public void onSave(Object entity) {
        replicateXmppSpecialUser(entity);
    }

    private void replicateXmppSpecialUser(Object entity) {
        if (entity instanceof User
                || (entity instanceof Group && ((Group) entity).getResource().equals(User.GROUP_RESOURCE_ID))) {
            getHibernateTemplate().flush();
            SpecialUser su = m_coreContext.getSpecialUserAsSpecialUser(SpecialUserType.XMPP_SERVER);
            if (su != null) {
                m_sipxReplicationContext.generate(m_coreContext
                        .getSpecialUserAsSpecialUser(SpecialUserType.XMPP_SERVER));
            }
        }
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    @Override
    public void onBeforeSave(Object entity) {
    }

    @Override
    public void onAfterDelete(Object entity) {
        replicateXmppSpecialUser(entity);
    }
}
