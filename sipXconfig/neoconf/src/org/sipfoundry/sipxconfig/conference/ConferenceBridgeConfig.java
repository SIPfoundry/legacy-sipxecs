/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.springframework.beans.factory.annotation.Required;

public class ConferenceBridgeConfig {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/conferencebridge-00-00";
    private DomainManager m_domainManager;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private AddressManager m_addressManager;

    public Document getDocument(Location location) {
        Document document = XmlFile.FACTORY.createDocument();
        Element confBridgeEl = document.addElement("conferencebridge", NAMESPACE);

        String domainName = m_domainManager.getDomain().getName();
        String ivrServiceHostPort = findMailboxServer();

        Bridge bridge = m_conferenceBridgeContext.getBridgeByServer(location.getFqdn());
        if (bridge == null) {
            return document;
        }

        for (Conference conference : bridge.getConferences()) {
            generateConference(confBridgeEl, conference, domainName, ivrServiceHostPort);
        }
        return document;
    }

    private String findMailboxServer() {
        return m_addressManager.getSingleAddress(Ivr.REST_API).addressColonPort();
    }

    private void generateConference(Element usersEl, Conference conference, String domainName,
            String ivrServiceHostPort) {
        User owner = conference.getOwner();
        if (owner == null) {
            return;
        }
        if (conference.isAutorecorded()) {
            Element userEl = usersEl.addElement("item");
            userEl.addElement("bridgename").setText(conference.getName());
            userEl.addElement("bridgecontact").setText(conference.getUri());
            userEl.addElement("extensionname").setText(conference.getExtension());
            userEl.addElement("ownername").setText(owner.getUserName());
            userEl.addElement("ownerid").setText(owner.getAddrSpec(domainName));
            userEl.addElement("mboxserver").setText(ivrServiceHostPort);
        }
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
