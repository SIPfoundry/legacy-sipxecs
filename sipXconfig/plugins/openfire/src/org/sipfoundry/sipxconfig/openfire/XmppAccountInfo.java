/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.util.Collection;
import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class XmppAccountInfo extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/xmpp-account-info-00-00";
    private static final String USER = "user";
    private static final String USER_NAME = "user-name";
    private CoreContext m_coreContext;

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element accountInfos = document.addElement("xmpp-account-info", NAMESPACE);
        List<User> users = m_coreContext.loadUsers();

        for (User user : users) {
            if (user.hasImAccount()) {
                createUserAccount(user, accountInfos);
            }
        }
        List<Group> groups = m_coreContext.getGroups();
        for (Group group : groups) {
            createXmmpGroup(group, accountInfos);
        }
        accountInfos.addElement("chat-room", NAMESPACE);

        return document;
    }

    private void createUserAccount(User user, Element accountInfos) {
        String displayName = user.getImDisplayName();
        if (null == displayName) {
            displayName = user.getName();
        }

        String onPhoneMessage = (String) user.getSettingTypedValue("openfire/on-the-phone-message");
        if (null == onPhoneMessage) {
            onPhoneMessage = "";
        }

        Element userAccounts = accountInfos.addElement(USER);
        userAccounts.addElement(USER_NAME).setText(user.getImId());
        userAccounts.addElement("sip-user-name").setText(user.getName());
        userAccounts.addElement("display-name").setText(displayName);
        userAccounts.addElement("password").setText(user.getImId());
        userAccounts.addElement("on-the-phone-message").setText(onPhoneMessage);
        userAccounts.addElement("advertise-on-call-status").setText(
                ((Boolean) user.getSettingTypedValue("openfire/advertise-sip-presence")).toString());
        userAccounts.addElement("show-on-call-details").setText(
                ((Boolean) user.getSettingTypedValue("openfire/include-call-info")).toString());
    }

    private void createXmmpGroup(Group group, Element accountInfos) {
        Element xmmpGroup = accountInfos.addElement("group");
        xmmpGroup.addElement("group-name").setText(group.getName());
        String groupDescription = group.getDescription();
        if (groupDescription != null) {
            xmmpGroup.addElement("description").setText(groupDescription);
        }
        Collection<User> groupMembers = m_coreContext.getGroupMembers(group);
        if (groupMembers != null && groupMembers.size() > 0) {
            for (User user : groupMembers) {
                Element userElement = xmmpGroup.addElement(USER);
                userElement.addElement(USER_NAME).setText(user.getName());
            }
        }
    }
}
