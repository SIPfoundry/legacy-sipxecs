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
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;
import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;
import static org.sipfoundry.sipxconfig.speeddial.SpeedDial.getResourceListId;

public class ResourceLists extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01";
    private CoreContext m_coreContext;

    private SpeedDialManager m_speedDialManager;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element lists = document.addElement("lists", NAMESPACE);
        forAllUsersDo(m_coreContext, new UserClosure(lists));
        return document;
    }

    private Element createResource(Element list, String uri, String name) {
        Element resource = list.addElement("resource");
        // Append "sipx-noroute=Voicemail" and "sipx-userforward=false"
        // URI parameters to the target URI to control how the proxy forwards
        // SUBSCRIBEs to the resource URI.
        resource.addAttribute("uri", uri + ";sipx-noroute=VoiceMail;sipx-userforward=false");
        addNameElement(resource, name);
        return resource;
    }

    Element createResourceForUser(Element list, Button button) {
        String name = StringUtils.defaultIfEmpty(button.getLabel(), button.getNumber());
        return createResource(list, buildUri(button, m_coreContext.getDomainName()), name);
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
        return uri.toString();
    }

    private void addNameElement(Element parent, String name) {
        parent.addElement("name").setText(name);
    }

    private Element createResourceList(Element lists, String name, String full, String consolidated) {
        Element list = lists.addElement("list");
        list.addAttribute("user", full);
        list.addAttribute("user-cons", consolidated);
        addNameElement(list, name);
        return list;
    }

    private Element createResourceList(Element lists, String name) {
        return createResourceList(lists, name, getResourceListId(name, false), getResourceListId(name, true));
    }

    private Element createResourceList(Element lists, SpeedDial speedDial) {
        return createResourceList(lists, speedDial.getResourceListName(), speedDial.getResourceListId(false),
                speedDial.getResourceListId(true));
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    private class UserClosure implements Closure<User> {
        private final String m_domainName = m_coreContext.getDomainName();
        private Element m_imList;
        private final Element m_lists;

        public UserClosure(Element lists) {
            m_lists = lists;
        }

        @Override
        public void execute(User user) {
            ImAccount imAccount = new ImAccount(user);
            if (imAccount.isEnabled()) {
                if (m_imList == null) {
                    m_imList = createResourceList(m_lists, XMPP_SERVER.getUserName());
                }
                createResource(m_imList, user.getAddrSpec(m_domainName), user.getName());
            }
            SpeedDial speedDial = m_speedDialManager.getSpeedDialForUserId(user.getId(), false);
            // ignore disabled orbits
            if (speedDial == null) {
                return;
            }

            // check if the user has the "Subscribe to Presence" permission since the blf flag
            // might not be set to FALSE even when the user doesn't have this permission(this could happen
            // when the user use the group's speed dial)
            if (!user.hasPermission(PermissionName.SUBSCRIBE_TO_PRESENCE)) {
                return;
            }

            List<Button> buttons = speedDial.getButtons();
            Element list = null;
            for (Button button : buttons) {
                if (!button.isBlf()) {
                    continue;
                }
                if (list == null) {
                    list = createResourceList(m_lists, speedDial);
                }
                createResourceForUser(list, button);
            }
        }
    }
}
