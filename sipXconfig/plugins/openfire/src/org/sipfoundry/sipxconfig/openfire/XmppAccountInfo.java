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

import java.util.List;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

public class XmppAccountInfo extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/xmpp-account-info-00-00";
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
        accountInfos.addElement("group", NAMESPACE);
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

        Element userAccounts = accountInfos.addElement("user");
        userAccounts.addElement("user-name").setText(user.getImId());
        userAccounts.addElement("sip-user-name").setText(user.getName());
        userAccounts.addElement("display-name").setText(displayName);
        userAccounts.addElement("password").setText(user.getImId());
        userAccounts.addElement("on-the-phone-message").setText(onPhoneMessage);
        userAccounts.addElement("advertise-on-call-status").setText(((Boolean) user.getSettingTypedValue(
                        "openfire/advertise-sip-presence")).toString());
        userAccounts.addElement("show-on-call-details").setText(((Boolean) user.getSettingTypedValue(
                        "openfire/include-call-info")).toString());

    }

}
