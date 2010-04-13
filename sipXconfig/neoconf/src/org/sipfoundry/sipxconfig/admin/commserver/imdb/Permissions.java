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

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class Permissions extends DataSetGenerator {
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

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                addUser(items, user, domain);
            }
        };
        forAllUsersDo(getCoreContext(), closure);

        List<InternalUser> internalUsers = getCoreContext().loadInternalUsers();
        for (InternalUser user : internalUsers) {
            addUser(items, user, domain);
        }
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
                String vmUri = SipUri.format(null, "~~vm~" + m_user.getName(), m_domain);
                addPermission(vmUri, name);
            }
        }

        private void addPermission(String uri, String permissionName) {
            Map<String, String> userItem = addItem(m_items);
            userItem.put("identity", uri);
            userItem.put("permission", permissionName);
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
