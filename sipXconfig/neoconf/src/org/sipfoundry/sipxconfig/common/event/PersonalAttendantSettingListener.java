/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common.event;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class PersonalAttendantSettingListener implements DaoEventListener {

    private static final String OPERATOR_SETTING = "personal-attendant" + Setting.PATH_DELIM + "operator";

    private MailboxManager m_mailboxManager;
    private JdbcTemplate m_jdbcTemplate;

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
        if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
            final String groupPa = group.getSettingValue(OPERATOR_SETTING);
            final List<String> sqlUpdates= new ArrayList<String>();
            m_jdbcTemplate.query("SELECT u.user_id, ug.group_id, v.value as user_pa from users u left join "
                    + "setting_value v on u.value_storage_id = v.value_storage_id "
                    + "AND v.path='personal-attendant/operator' "
                    + "inner join user_group ug on u.user_id = ug.user_id "
                    + "WHERE group_id=" + group.getId() + " AND u.user_type='C' "
                    + "ORDER BY u.user_id;", new RowCallbackHandler() {
                        @Override
                        public void processRow(ResultSet rs) throws SQLException {
                            String userSetPa = StringUtils.defaultIfEmpty(rs.getString("user_pa"), groupPa);
                            sqlUpdates.add("UPDATE personal_attendant SET operator='" 
                                    + userSetPa +"' WHERE user_id=" + rs.getInt("user_id") + ";");
                        }
                    });
            if (!sqlUpdates.isEmpty()) {
                m_jdbcTemplate.batchUpdate(sqlUpdates.toArray(new String[sqlUpdates.size()]));
            }
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

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }
}
