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

import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.springframework.dao.support.DataAccessUtils;

public class SipxParkService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxParkService";

    public static final String LOG_SETTING = "park-config/SIP_PARK_LOG_LEVEL";

    private String m_parkServerSipPort;

    private LocationsManager m_locationsManager;

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public String getParkServerSipPort() {
        return m_parkServerSipPort;
    }

    public void setParkServerSipPort(String parkServerSipPort) {
        m_parkServerSipPort = parkServerSipPort;
    }

    public String getAddress() {
        List<Location> locations = m_locationsManager.getLocationsForService(this);
        Location location = (Location) DataAccessUtils.singleResult(locations);
        return null == location ? "" : location.getAddress();
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
