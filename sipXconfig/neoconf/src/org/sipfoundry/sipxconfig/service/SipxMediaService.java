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

public class SipxMediaService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxMediaService";

    public static final String LOG_SETTING = "mediaserver-config/SIPX_MEDIA_SERVER_LOG_LEVEL";

    private int m_httpPort;
    private int m_httpsPort;
    private LocationsManager m_locationsManager;
    private String m_mediaServerSipSrvOrHostport;

    public String getVoicemailServer() {
        StringBuffer voicemailServer = new StringBuffer();
        voicemailServer.append("https://");
        voicemailServer.append(getMediaServerAddress());
        if (getVoicemailHttpsPort() != 0) {
            voicemailServer.append(':');
            voicemailServer.append(getVoicemailHttpsPort());
        }

        return voicemailServer.toString();
    }

    public String getMediaServer() {
        return getMediaServerSipSrvOrHostport() + ";transport=tcp";
    }

    public String getMediaServerSipSrvOrHostport() {
        if (m_mediaServerSipSrvOrHostport == null) {
            StringBuffer sipSrvOrHostportBuffer = new StringBuffer();
            sipSrvOrHostportBuffer.append(getMediaServerAddress());
            if (getSipPort() != null) {
                sipSrvOrHostportBuffer.append(':');
                sipSrvOrHostportBuffer.append(getSipPort());
            }

            m_mediaServerSipSrvOrHostport = sipSrvOrHostportBuffer.toString();
        }

        return m_mediaServerSipSrvOrHostport;
    }

    public int getVoicemailHttpPort() {
        return m_httpPort;
    }

    public void setVoicemailHttpPort(int httpPort) {
        m_httpPort = httpPort;
    }

    public int getVoicemailHttpsPort() {
        return m_httpsPort;
    }

    public void setVoicemailHttpsPort(int httpsPort) {
        m_httpsPort = httpsPort;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    private String getMediaServerAddress() {
        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        if (primaryLocation == null) {
            return "localhost";
        }
        return primaryLocation.getAddress();
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
