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
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.dao.support.DataAccessUtils;

public class SipxStatusService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxStatusService";

    public static final String LOG_SETTING = "status-config/SIP_STATUS_LOG_LEVEL";

    // TODO: remove once it's removed from config.defs and voicemail.xml
    private int m_httpsPort;

    private LocationsManager m_locationsManager;

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setHttpsPort(int httpsPort) {
        m_httpsPort = httpsPort;
    }

    public int getHttpsPort() {
        return m_httpsPort;
    }

    @Override
    public String getSipPort() {
        Setting statusSettings = getSettings().getSetting("status-config");
        return statusSettings.getSetting("SIP_STATUS_SIP_PORT").getValue();
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
