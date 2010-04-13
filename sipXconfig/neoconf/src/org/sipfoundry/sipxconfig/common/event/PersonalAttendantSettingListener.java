/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common.event;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public class PersonalAttendantSettingListener implements DaoEventListener {

    private static final String OPERATOR_SETTING = "personal-attendant" + Setting.PATH_DELIM + "operator";

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
            String operator = user.getSettingValue(OPERATOR_SETTING);
            m_mailboxManager.updatePersonalAttendantForUser(user, operator);
        }
    }

    /**
     * Updates the personal attendant for a user.
     */
    private void onSaveUser(User user) {
        if (user.isNew()) {
            // nothing to update yet
            return;
        }
        // HACK: permission manager can be empty during tests - and settings are not loaded
        if (user.getSettings() == null) {
            return;
        }
        String operator = user.getSettingValue(OPERATOR_SETTING);
        m_mailboxManager.updatePersonalAttendantForUser(user, operator);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
