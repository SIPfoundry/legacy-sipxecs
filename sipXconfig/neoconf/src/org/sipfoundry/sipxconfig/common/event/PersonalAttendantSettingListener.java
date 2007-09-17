/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common.event;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingValue;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public class PersonalAttendantSettingListener implements DaoEventListener {

    private static final String OPERATOR_SETTING = 
        "personal-attendant" + Setting.PATH_DELIM + "operator";
    
    private CoreContext m_coreContext;
    private MailboxManager m_mailboxManager;
    
    public void onDelete(Object entity) {
        // nothing to do on delete
    }

    public void onSave(Object entity) {
        if (Group.class == entity.getClass()) {
            onSaveGroup((Group) entity);
        } else if (User.class == entity.getClass()) {
            onSaveUser((User) entity);
        }
    }

    private void onSaveGroup(Group group) {
        Collection<User> usersInGroup = m_coreContext.getGroupMembers(group);
        for (User user : usersInGroup) {
            Setting operatorSetting = user.getSettings().getSetting(OPERATOR_SETTING);
            String operatorValue = operatorSetting.getValue();
            updatePersonalAttendantForUser(user, operatorValue);
        }
    }
    
    /**
     * Updates the personal attendant for a user given that either a) the 
     * user has settings associated with it (i.e. user.getSettings != null) or 
     * b) the user's group has settings associated with it.
     * @param user
     */
    private void onSaveUser(User user) {
        List<Group> groupsForUser = user.getGroupsAsList();
        Group group = Group.selectGroupWithHighestWeight(groupsForUser);
        if (group != null) {
            SettingValue operatorSetting = group.getSettingValue(new SettingImpl(OPERATOR_SETTING));
            if (operatorSetting != null) {
                updatePersonalAttendantForUser(user, operatorSetting.getValue());
            }
        }
    }
    
    private void updatePersonalAttendantForUser(User user, String operatorValue) {
        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(user);
        pa.setOperator(operatorValue);
        m_mailboxManager.storePersonalAttendant(pa);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
