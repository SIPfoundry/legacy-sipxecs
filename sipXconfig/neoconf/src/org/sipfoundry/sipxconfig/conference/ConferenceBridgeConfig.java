/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxRecordingService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class ConferenceBridgeConfig extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/conferencebridge-00-00";

    private DomainManager m_domainManager;

    private ConferenceBridgeContext m_conferenceBridgeContext;

    private SipxServiceManager m_sipxServiceManager;

    @Override
    public Document getDocument(Location location) {
        Document document = FACTORY.createDocument();
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

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxRecordingService.BEAN_ID);
    }

    private String findMailboxServer() {
        SipxIvrService ivrService = (SipxIvrService) m_sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        String fqdn = ivrService.getFqdn();
        if (fqdn == null) {
            return StringUtils.EMPTY;
        }
        return fqdn + ":" + ivrService.getHttpsPort();
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

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }
}
