/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.springframework.beans.factory.annotation.Required;

public class SipxCallResolverService extends SipxService implements LoggingEntity {

    public static final String BEAN_ID = "sipxCallResolverService";

    public static final String LOG_SETTING = "callresolver/SIP_CALLRESOLVER_LOG_LEVEL";

    private int m_agentPort;
    private LocationsManager m_locationManager;

    public int getAgentPort() {
        return m_agentPort;
    }

    public void setAgentPort(int agentPort) {
        m_agentPort = agentPort;
    }

    public String getAgentAddress() {
        Location agentLocation = m_locationManager.getPrimaryLocation();
        return agentLocation.getAddress();
    }

    @Required
    public void setLocationManager(LocationsManager locationManager) {
        m_locationManager = locationManager;
    }
    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }
}
