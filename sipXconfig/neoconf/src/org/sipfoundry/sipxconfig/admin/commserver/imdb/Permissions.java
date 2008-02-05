/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

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
    protected void addItems(Element items) {
        String domain = getSipDomain();
        for (SpecialUserType sut : SpecialUserType.values()) {
            addSpecialUser(sut.getUserName(), items, domain);
        }

        List<CallGroup> callGroups = m_callGroupContext.getCallGroups();
        for (CallGroup callGroup : callGroups) {
            addSpecialUser(callGroup.getName(), items, domain);
        }

        List<User> users = getCoreContext().loadUsers();
        for (User user : users) {
            addUser(items, user, domain);
        }
    }

    void addUser(Element item, User user, String domain) {
        Setting permissions = user.getSettings().getSetting(Permission.CALL_PERMISSION_PATH);
        Setting voicemailPermissions = user.getSettings().getSetting(
                Permission.VOICEMAIL_SERVER_PATH);
        PermissionWriter writer = new PermissionWriter(user, domain, item);
        permissions.acceptVisitor(writer);
        voicemailPermissions.acceptVisitor(writer);
    }

    private void addSpecialUser(String userId, Element items, String domain) {
        User user = getCoreContext().newUser();
        user.setPermission(PermissionName.VOICEMAIL, false);
        user.setPermission(PermissionName.SIPX_VOICEMAIL, false);
        user.setPermission(PermissionName.EXCHANGE_VOICEMAIL, false);
        user.setUserName(userId);
        addUser(items, user, domain);
    }

    class PermissionWriter extends AbstractSettingVisitor {
        private User m_user;

        private Element m_items;

        private String m_domain;

        PermissionWriter(User user, String domain, Element items) {
            m_user = user;
            m_items = items;
            m_domain = domain;
        }

        public void visitSetting(Setting setting) {
            if (!Permission.isEnabled(setting.getValue())) {
                return;
            }
            String name = setting.getName();
            String uri = m_user.getUri(m_domain);
            addPermission(uri, name);

            // add special permission for voicemail redirect rule for xcf-1875
            if (name.equals(PermissionName.EXCHANGE_VOICEMAIL.getName())
                    || name.equals(PermissionName.SIPX_VOICEMAIL.getName())) {
                String vmUri = SipUri.format(null, "~~vm~" + m_user.getName(), m_domain);
                addPermission(vmUri, name);
            }
        }

        private void addPermission(String uri, String permissionName) {
            Element userItem = addItem(m_items);
            userItem.addElement("identity").setText(uri);
            userItem.addElement("permission").setText(permissionName);
        }
    }

    protected DataSet getType() {
        return DataSet.PERMISSION;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
