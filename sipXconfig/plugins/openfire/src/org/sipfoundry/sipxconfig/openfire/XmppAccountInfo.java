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
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.defaultString;

public class XmppAccountInfo extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/xmpp-account-info-00-00";
    private static final String USER = "user";
    private static final String USER_NAME = "user-name";
    private static final String PASSWORD = "password";
    private static final String DESCRIPTION = "description";
    private CoreContext m_coreContext;
    private ConferenceBridgeContext m_conferenceContext;

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceContext) {
        m_conferenceContext = conferenceContext;
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

        List<Conference> conferences = m_conferenceContext.getAllConferences();
        for (Conference conf : conferences) {
            createXmppChatRoom(conf, accountInfos);
        }
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
        userAccounts.addElement(PASSWORD).setText(user.getImId());
        userAccounts.addElement("on-the-phone-message").setText(onPhoneMessage);
        userAccounts.addElement("advertise-on-call-status").setText(
                ((Boolean) user.getSettingTypedValue("openfire/advertise-sip-presence")).toString());
        userAccounts.addElement("show-on-call-details").setText(
                ((Boolean) user.getSettingTypedValue("openfire/include-call-info")).toString());
    }

    private void createXmppChatRoom(Conference conference, Element accountInfos) {
        if (!conference.isEnabled()) {
            return;
        }
        User owner = conference.getOwner();
        if (owner == null) {
            return;
        }
        if (!owner.hasImAccount()) {
            return;
        }

        Element chatRoom = accountInfos.addElement("chat-room");

        String displayName = conference.getName();

        chatRoom.addElement("subdomain").setText(conference.getBridge().getHost());
        chatRoom.addElement("room-owner").setText(owner.getImId());
        chatRoom.addElement("room-name").setText(displayName);
        chatRoom.addElement(DESCRIPTION).setText(defaultString(conference.getDescription()));
        chatRoom.addElement(PASSWORD).setText(defaultString(conference.getParticipantAccessCode()));
        addBooleanSetting(chatRoom, conference, "moderated", "chat-meeting/moderated");
        addBooleanSetting(chatRoom, conference, "is-public-room", "chat-meeting/public");
        addBooleanSetting(chatRoom, conference, "is-members-only", "chat-meeting/members-only");
        addBooleanSetting(chatRoom, conference, "is-persistant", "chat-meeting/persistent");
        addBooleanSetting(chatRoom, conference, "is-room-listed", "chat-meeting/listed");
        addBooleanSetting(chatRoom, conference, "log-room-conversations", "chat-meeting/log-conversations");
        chatRoom.addElement("conference-extension").setText(conference.getExtension());
    }

    private void addBooleanSetting(Element chatRoom, Conference conference, String elementName, String settingName) {
        Boolean value = (Boolean) conference.getSettingTypedValue(settingName);
        chatRoom.addElement(elementName).setText(value.toString());
    }

    private void createXmmpGroup(Group group, Element accountInfos) {
        Element xmmpGroup = accountInfos.addElement("group");
        xmmpGroup.addElement("group-name").setText(group.getName());
        String groupDescription = group.getDescription();
        if (groupDescription != null) {
            xmmpGroup.addElement(DESCRIPTION).setText(groupDescription);
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
