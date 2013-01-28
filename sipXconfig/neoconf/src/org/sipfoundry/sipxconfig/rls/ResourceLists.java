/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import static org.sipfoundry.commons.mongo.MongoConstants.BUTTONS;
import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.SPEEDDIAL;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;
import static org.sipfoundry.commons.mongo.MongoConstants.URI;
import static org.sipfoundry.commons.mongo.MongoConstants.USER;
import static org.sipfoundry.commons.mongo.MongoConstants.USER_CONS;
import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;
import static org.sipfoundry.sipxconfig.speeddial.SpeedDial.getResourceListId;

import java.util.Iterator;

import org.apache.commons.lang.BooleanUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.annotation.Required;

import com.mongodb.BasicDBList;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

public class ResourceLists {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01";
    private CoreContext m_coreContext;
    private ValidUsers m_validUsers;

    public Document getDocument(boolean xmppPresenceEnabled) {
        Document document = XmlFile.FACTORY.createDocument();
        Element lists = document.addElement("lists", NAMESPACE);
        Element imList = null;

        DBCursor cursor = m_validUsers.getUsersWithSpeedDial();
        while (cursor.hasNext()) {
            DBObject user = cursor.next();
            String userName = user.get(UID).toString();
            if (userName.equals("superadmin")) {
                continue;
            }
            if (xmppPresenceEnabled && BooleanUtils.toBoolean(user.get(IM_ENABLED).toString())) {
                if (imList == null) {
                    imList = createResourceList(lists, XMPP_SERVER.getUserName());
                }
                String userAddrSpec = SipUri.format(userName, m_coreContext.getDomainName(), false);
                createResource(imList, userAddrSpec, userName);
            }
            DBObject speedDial = (DBObject) user.get(SPEEDDIAL);

            // ignore disabled orbits
            if (speedDial == null) {
                continue;
            }

            BasicDBList buttons = (BasicDBList) speedDial.get(BUTTONS);
            Element list = null;
            if (buttons != null) {
                list = createResourceList(lists, user.get(UID).toString(), speedDial.get(USER).toString(),
                        speedDial.get(USER_CONS).toString());
                Iterator iter = buttons.iterator();
                while (iter.hasNext()) {
                    DBObject button = (DBObject) iter.next();
                    // Append "sipx-noroute=Voicemail" and "sipx-userforward=false"
                    // URI parameters to the target URI to control how the proxy forwards
                    // SUBSCRIBEs to the resource URI.
                    createResource(list, button.get(URI).toString(), button.get(NAME).toString());
                }
            }
        }
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

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public ValidUsers getValidUsers() {
        return m_validUsers;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }
}
