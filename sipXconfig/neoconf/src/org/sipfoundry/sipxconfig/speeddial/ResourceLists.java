/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

public class ResourceLists extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01";

    private CoreContext m_coreContext;

    private SpeedDialManager m_speedDialManager;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element lists = document.addElement("lists", NAMESPACE);
        List<User> users = m_coreContext.loadUsers();
        for (User user : users) {
            SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(user.getId(), false);
            // ignore disabled orbits
            if (speedDial == null) {
                continue;
            }
            List<Button> buttons = speedDial.getButtons();
            Element list = null;
            for (Button button : buttons) {
                if (!button.isBlf()) {
                    continue;
                }
                if (list == null) {
                    list = createListForUser(lists, speedDial);
                }
                createResourceForUser(list, button, m_coreContext.getDomainName());
            }
        }
        return document;
    }

    Element createResourceForUser(Element list, Button button, String domainName) {
        Element resource = list.addElement("resource");
        resource.addAttribute("uri", buildUri(button, domainName));
        addNameElement(resource, StringUtils.defaultIfEmpty(button.getLabel(), button.getNumber()));
        return resource;
    }

    String buildUri(Button button, String domainName) {
        String number = button.getNumber();
        StringBuilder uri = new StringBuilder();
        if (SipUri.matches(number)) {
            uri.append(SipUri.normalize(number));
        } else {
            // not a URI - check if we have a user
            User user = m_coreContext.loadUserByAlias(number);
            if (user != null) {
                // if number matches any known user make sure to use username and not an alias
                number = user.getUserName();
            }
            uri.append(SipUri.format(number, domainName, false));
        }
        // Append "sipx-noroute=Voicemail" and "sipx-userforward=false"
        // URI parameters to the target URI to control how the proxy forwards
        // SUBSCRIBEs to the resource URI.
        uri.append(";sipx-noroute=VoiceMail;sipx-userforward=false");
        return uri.toString();
    }

    private void addNameElement(Element parent, String name) {
        parent.addElement("name").setText(name);
    }

    private Element createListForUser(Element lists, SpeedDial speedDial) {
        Element list = lists.addElement("list");
        list.addAttribute("user", speedDial.getResourceListId(false));
        list.addAttribute("user-cons", speedDial.getResourceListId(true));
        addNameElement(list, speedDial.getResourceListName());
        return list;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }
}
