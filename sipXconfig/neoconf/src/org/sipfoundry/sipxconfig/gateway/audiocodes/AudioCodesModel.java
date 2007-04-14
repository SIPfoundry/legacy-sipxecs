/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.gateway.GatewayModel;

public class AudioCodesModel extends GatewayModel {
    private String m_profileTemplate;
    private String m_proxyNameSetting;
    private String m_proxyIpSetting;

    public void setProfileTemplate(String profileTemplate) {
        m_profileTemplate = profileTemplate;
    }

    public String getProfileTemplate() {
        return m_profileTemplate;
    }

    public String getProxyIpSetting() {
        return m_proxyIpSetting;
    }

    public void setProxyIpSetting(String proxyIpSetting) {
        m_proxyIpSetting = proxyIpSetting;
    }

    public String getProxyNameSetting() {
        return m_proxyNameSetting;
    }

    public void setProxyNameSetting(String proxyNameSetting) {
        m_proxyNameSetting = proxyNameSetting;
    }
}
