/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.acccode;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class AuthCodesConfig extends XmlFile {
    // please note: US locale always...
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/authcodes-00-00";
    private static final String ID = "id";
    private static final String PARAMETER = "parameter";
    private static final Log LOG = LogFactory.getLog(AuthCodesConfig.class);

    private DialPlanContext m_dialPlanContext;

    private DomainManager m_domainManager;

    private SipxServiceManager m_sipxServiceManager;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        QName authcodesName = FACTORY.createQName("authcodes", NAMESPACE);
        Element aasEl = document.addElement(authcodesName);
        return document;
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxIvrService.BEAN_ID);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    private void logNullParameterError() {
        LOG.warn("Menu item's parameter is null. "
                + "The generation of autoattendants.xml file will ignore this parameter and continue.");
    }
}
