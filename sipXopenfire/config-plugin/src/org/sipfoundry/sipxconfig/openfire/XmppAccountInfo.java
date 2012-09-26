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

import static org.apache.commons.lang.StringUtils.defaultString;

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.imbot.ImBotSettings;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

public class XmppAccountInfo {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/xmpp-account-info-00-00";
    private static final String MUC_SUBDOMAIN = "conference";
    private static final String USER = "user";
    private static final String USER_NAME = "user-name";
    private static final String PASSWORD = "password";
    private static final String DESCRIPTION = "description";
    private static final String DISPLAY_NAME = "display-name";

    private CoreContext m_coreContext;
    private ConferenceBridgeContext m_conferenceContext;
    private ValidUsers m_validUsers;
    private ImBot m_imbot;

    public Document getDocument() {
        ImBotSettings imbotSettings = m_imbot.getSettings();
        Document document = XmlFile.FACTORY.createDocument();
        final Element accountInfos = document.addElement("xmpp-account-info", NAMESPACE);

        createPaUserAccount(accountInfos, imbotSettings);

        List<Group> groups = m_coreContext.getGroups();
        for (Group group : groups) {
            createXmmpGroup(group, accountInfos, imbotSettings);
        }

        return document;
    }

    private void createPaUserAccount(Element accountInfos, ImBotSettings imbotSettings) {
        String paUserName = imbotSettings.getPersonalAssistantImId();
        String paPassword = imbotSettings.getPersonalAssistantImPassword();

        User paUser = m_coreContext.newUser();
        paUser.setUserName(paUserName);
        ImAccount imAccount = new ImAccount(paUser);
        imAccount.setEnabled(true);

        createUserAccount(paUser, accountInfos, paPassword);
    }

    private void createUserAccount(User user, Element accountInfos, String paPassword) {
        ImAccount imAccount = new ImAccount(user);
        if (!imAccount.isEnabled()) {
            return;
        }

        Element userAccounts = accountInfos.addElement(USER);
        userAccounts.addElement(USER_NAME).setText(imAccount.getImId());
        userAccounts.addElement("sip-user-name").setText(user.getName());
        userAccounts.addElement(DISPLAY_NAME).setText(imAccount.getImDisplayName());
        userAccounts.addElement(PASSWORD).setText(paPassword);
        String email = imAccount.getEmailAddress();
        userAccounts.addElement("email").setText(email != null ? email : "");
        userAccounts.addElement("on-the-phone-message").setText(imAccount.getOnThePhoneMessage());
        userAccounts.addElement("advertise-on-call-status").setText(
                Boolean.toString(imAccount.advertiseSipPresence()));
        userAccounts.addElement("show-on-call-details").setText(Boolean.toString(imAccount.includeCallInfo()));
    }

    private void addBooleanSetting(Element chatRoom, Conference conference, String elementName, String settingName) {
        Boolean value = (Boolean) conference.getSettingTypedValue(settingName);
        chatRoom.addElement(elementName).setText(value.toString());
    }

    private void createXmmpGroup(Group group, Element accountInfos, ImBotSettings settings) {
        // HACK: we assume that 'replicate-group' is a standard boolean setting
        Boolean replicate = (Boolean) group.getSettingTypedValue(new BooleanSetting(), "im/im-group");
        if (replicate == null || !replicate) {
            return;
        }

        Element xmmpGroup = accountInfos.addElement("group");
        xmmpGroup.addElement("group-name").setText(group.getName());
        String groupDescription = group.getDescription();
        if (groupDescription != null) {
            xmmpGroup.addElement(DESCRIPTION).setText(groupDescription);
        }

        //use mongo here since it's much faster
        //downside: in case of bulk generation of users (send profiles, group)
        //file must be generated after all users are generated
        List<String> imIds = m_validUsers.getAllImIdsInGroup(group.getName());
        for (String imId : imIds) {
            Element userElement = xmmpGroup.addElement(USER);
            userElement.addElement(USER_NAME).setText(imId);
        }

        Boolean addPersonalAssistant = (Boolean) group.getSettingTypedValue(new BooleanSetting(),
                "im/add-pa-to-group");
        if (addPersonalAssistant != null && addPersonalAssistant) {
            Element userElement = xmmpGroup.addElement(USER);
            String paUserName = settings.getPersonalAssistantImId();
            userElement.addElement(USER_NAME).setText(paUserName);
        }
    }

    public ValidUsers getValidUsers() {
        return m_validUsers;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setConferenceContext(ConferenceBridgeContext conferenceContext) {
        m_conferenceContext = conferenceContext;
    }

    public void setImbot(ImBot imbot) {
        m_imbot = imbot;
    }
}
