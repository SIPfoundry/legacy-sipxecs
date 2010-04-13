/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;


public class SipTrunkModel extends GatewayModel {

    public static final String TEMPLATE_NONE = "None";

    private String m_itspTemplate = "siptrunk.xml";
    private String m_templateLocation = "commserver";
    private String m_itspName = TEMPLATE_NONE;

    public SipTrunkModel() {
        setBeanId(SipTrunk.BEAN_ID);
    }

    public void setItspTemplate(String itspTemplate) {
        m_itspTemplate = itspTemplate;
    }

    public String getItspTemplate() {
        return m_itspTemplate;
    }

    public void setTemplateLocation(String templateLocation) {
        m_templateLocation = templateLocation;
    }

    public String getTemplateLocation() {
        return m_templateLocation;
    }

    public void setItspName(String itspName) {
        m_itspName = itspName;
    }

    public String getItspName() {
        return m_itspName;
    }

    /** SIP trunks don't have serial numbers, so return false */
    @Override
    public boolean getHasSerialNumber() {
        return false;
    }

}
