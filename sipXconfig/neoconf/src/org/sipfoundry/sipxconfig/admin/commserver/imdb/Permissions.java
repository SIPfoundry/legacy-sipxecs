/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.jdbc.core.RowCallbackHandler;

public class Permissions extends DataSetGenerator {
    private static final String QUERY = "SELECT u.user_id, u.first_name, u.last_name, u.user_name, "
            + "(SELECT count(*) from user_group where user_id = u.user_id) as groups, "
            + "v.value as nineoodialing, sv.value as autoattendant, sp.value as international, "
            + "si.value as local, sd.value as long_distance, sm.value as mobile, sr.value as record_prompts, "
            + "st.value as toll_free, svm.value as voicemail, se.value as exchange, sf.value as freeswitch "
            + "FROM users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='permission/call-handling/900Dialing' "
            + "left join setting_value sv on u.value_storage_id = sv.value_storage_id "
            + "AND sv.path='permission/call-handling/AutoAttendant' "
            + "left join setting_value sp on u.value_storage_id = sp.value_storage_id "
            + "AND sp.path='permission/call-handling/InternationalDialing' "
            + "left join setting_value si on u.value_storage_id = si.value_storage_id "
            + "AND si.path='permission/call-handling/LocalDialing' "
            + "left join setting_value sd on u.value_storage_id = sd.value_storage_id "
            + "AND sd.path='permission/call-handling/LongDistanceDialing' "
            + "left join setting_value sm on u.value_storage_id = sm.value_storage_id "
            + "AND sm.path='permission/call-handling/Mobile' "
            + "left join setting_value sr on u.value_storage_id = sr.value_storage_id "
            + "AND sr.path='permission/call-handling/RecordSystemPrompts' "
            + "left join setting_value st on u.value_storage_id = st.value_storage_id "
            + "AND st.path='permission/call-handling/TollFree' "
            + "left join setting_value svm on u.value_storage_id = svm.value_storage_id "
            + "AND svm.path='permission/call-handling/Voicemail' "
            + "left join setting_value se on u.value_storage_id = se.value_storage_id "
            + "AND se.path='permission/voicemail-server/ExchangeUMVoicemailServer' "
            + "left join setting_value sf on u.value_storage_id = sf.value_storage_id "
            + "AND sf.path='permission/voicemail-server/FreeswitchVoicemailServer' "
            + "WHERE u.user_type='C' ORDER BY u.user_id;";
    private static final String FREESWITCH = "freeswitch";
    private static final String EXCHANGE = "exchange";
    private static final String IDENTITY = "identity";
    private static final String PERMISSION = "permission";
    private CallGroupContext m_callGroupContext;

    /**
     * Adds: <code>
     * <item>
     *   <identity>user_uri</identity>
     *   <permission>permission_name</permission>
     * </item>
     * </code>
     *
     * to the list of items.
     */
    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domain = getSipDomain();
        for (SpecialUserType sut : SpecialUserType.values()) {

            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!sut.equals(SpecialUserType.PHONE_PROVISION)) {
                addSpecialUser(sut.getUserName(), items, domain);
            }
        }

        List<CallGroup> callGroups = m_callGroupContext.getCallGroups();
        for (CallGroup callGroup : callGroups) {
            if (callGroup.isEnabled()) {
                addSpecialUser(callGroup.getName(), items, domain);
            }
        }

        // user to retrieve default permission values
        final User userModel = getCoreContext().newUser();

        List<Group> groups = getCoreContext().getGroups();
        final Map<Integer, Group> groupsMap = new HashMap<Integer, Group>();
        for (Group group : groups) {
            groupsMap.put(group.getId(), group);
        }
        getJdbcTemplate().query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String userId = rs.getString("user_id");
                String userName = rs.getString("user_name");
                String firstName = rs.getString("first_name");
                String lastName = rs.getString("last_name");
                Object[] names = {
                    firstName, lastName
                };
                String displayName = StringUtils.trimToNull(StringUtils.join(names, ' '));

                int groupsCount = rs.getInt("groups");
                userModel.setGroups(new TreeSet<Group>());
                if (groupsCount > 0) {
                    // add groups to this user model
                    getJdbcTemplate().query("SELECT u.group_id from user_group u inner join group_storage s "
                            + "on u.group_id = s.group_id WHERE user_id=" + userId
                            + " AND s.resource='user';", new RowCallbackHandler() {

                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    userModel.addGroup(groupsMap.get(rs.getInt("group_id")));
                                }
                            });
                }
                // add call handling permissions
                String uri = SipUri.format(displayName, userName, domain);
                addPermission(items, rs, "nineoodialing",
                        getDefaultCallHandlingValue(userModel, PermissionName.NINEHUNDERED_DIALING),
                        PermissionName.NINEHUNDERED_DIALING.getName(), uri);
                addPermission(items, rs, "autoattendant",
                        getDefaultCallHandlingValue(userModel, PermissionName.AUTO_ATTENDANT_DIALING),
                        PermissionName.AUTO_ATTENDANT_DIALING.getName(), uri);
                addPermission(items, rs, "international",
                        getDefaultCallHandlingValue(userModel, PermissionName.INTERNATIONAL_DIALING),
                        PermissionName.INTERNATIONAL_DIALING.getName(), uri);
                addPermission(items, rs, "local",
                        getDefaultCallHandlingValue(userModel, PermissionName.LOCAL_DIALING),
                        PermissionName.LOCAL_DIALING.getName(), uri);
                addPermission(items, rs, "long_distance",
                        getDefaultCallHandlingValue(userModel, PermissionName.LONG_DISTANCE_DIALING),
                        PermissionName.LONG_DISTANCE_DIALING.getName(), uri);
                addPermission(items, rs, "mobile", getDefaultCallHandlingValue(userModel, PermissionName.MOBILE),
                        PermissionName.MOBILE.getName(), uri);
                addPermission(items, rs, "toll_free",
                        getDefaultCallHandlingValue(userModel, PermissionName.TOLL_FREE_DIALING),
                        PermissionName.TOLL_FREE_DIALING.getName(), uri);
                addPermission(items, rs, "voicemail",
                        getDefaultCallHandlingValue(userModel, PermissionName.VOICEMAIL),
                        PermissionName.VOICEMAIL.getName(), uri);
                addPermission(items, rs, "record_prompts",
                        getDefaultCallHandlingValue(userModel, PermissionName.RECORD_SYSTEM_PROMPTS),
                        PermissionName.RECORD_SYSTEM_PROMPTS.getName(), uri);

                // add voicemail server permissions
                String vmUri = getVmUri(userName, domain);
                String fsDefault = getDefaultVoicemailValue(userModel, PermissionName.FREESWITH_VOICEMAIL);
                String exchangeDefault = getDefaultVoicemailValue(userModel, PermissionName.EXCHANGE_VOICEMAIL);
                addPermission(items, rs, FREESWITCH, fsDefault, PermissionName.FREESWITH_VOICEMAIL.getName(), uri);
                addPermission(items, rs, FREESWITCH, fsDefault, PermissionName.FREESWITH_VOICEMAIL.getName(), vmUri);
                addPermission(items, rs, EXCHANGE, exchangeDefault, PermissionName.EXCHANGE_VOICEMAIL.getName(), uri);
                addPermission(items, rs, EXCHANGE, exchangeDefault, PermissionName.EXCHANGE_VOICEMAIL.getName(),
                        vmUri);
            }
        });

        List<InternalUser> internalUsers = getCoreContext().loadInternalUsers();
        for (InternalUser user : internalUsers) {
            addUser(items, user, domain);
        }
    }

    private String getDefaultCallHandlingValue(User user, PermissionName permission) {
        return getDefaultValue(user, Permission.CALL_PERMISSION_PATH, permission);
    }

    private String getDefaultVoicemailValue(User user, PermissionName permission) {
        return getDefaultValue(user, Permission.VOICEMAIL_SERVER_PATH, permission);
    }

    private String getDefaultValue(User user, String path, PermissionName permission) {
        return user.getSettings().getSetting(path + "/" + permission.getName()).getDefaultValue();
    }

    void addUserPermission(List<Map<String, String>> items, String uri, String permission) {
        Map<String, String> item = addItem(items);
        item.put(IDENTITY, uri);
        item.put(PERMISSION, permission);
    }

    void addUser(List<Map<String, String>> item, AbstractUser user, String domain) {
        Setting permissions = user.getSettings().getSetting(Permission.CALL_PERMISSION_PATH);
        Setting voicemailPermissions = user.getSettings().getSetting(Permission.VOICEMAIL_SERVER_PATH);
        PermissionWriter writer = new PermissionWriter(user, domain, item);
        permissions.acceptVisitor(writer);
        voicemailPermissions.acceptVisitor(writer);
    }

    private void addSpecialUser(String userId, List<Map<String, String>> items, String domain) {
        User user = getCoreContext().newUser();
        user.setPermission(PermissionName.VOICEMAIL, false);
        user.setPermission(PermissionName.FREESWITH_VOICEMAIL, false);
        user.setPermission(PermissionName.EXCHANGE_VOICEMAIL, false);
        user.setUserName(userId);
        addUser(items, user, domain);
    }

    private void addPermission(List<Map<String, String>> items, ResultSet rs, String columnName,
            String defaultValue, String permission, String uri) throws SQLException {
        String permissionValue = StringUtils.defaultIfEmpty(rs.getString(columnName), defaultValue);
        if (permissionValue.equals(Permission.ENABLE)) {
            addUserPermission(items, uri, permission);
        }
    }

    private String getVmUri(String userName, String domain) {
        return SipUri.format(null, "~~vm~" + userName, domain);
    }

    class PermissionWriter extends AbstractSettingVisitor {
        private final AbstractUser m_user;

        private final List<Map<String, String>> m_items;

        private final String m_domain;

        PermissionWriter(AbstractUser user, String domain, List<Map<String, String>> items) {
            m_user = user;
            m_items = items;
            m_domain = domain;
        }

        @Override
        public void visitSetting(Setting setting) {
            if (!Permission.isEnabled(setting.getValue())) {
                return;
            }
            String name = setting.getName();
            String uri = m_user.getUri(m_domain);
            addPermission(uri, name);

            // add special permission for voicemail redirect rule for xcf-1875
            if (name.equals(PermissionName.EXCHANGE_VOICEMAIL.getName())
                    || name.equals(PermissionName.FREESWITH_VOICEMAIL.getName())) {
                addPermission(getVmUri(m_user.getName(), m_domain), name);
            }
        }

        private void addPermission(String uri, String permissionName) {
            Map<String, String> userItem = addItem(m_items);
            userItem.put(IDENTITY, uri);
            userItem.put(PERMISSION, permissionName);
        }
    }

    @Override
    protected DataSet getType() {
        return DataSet.PERMISSION;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
