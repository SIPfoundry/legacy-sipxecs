/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class NatLocation {
    private boolean m_useStun = true;

    private String m_stunAddress = "stun01.sipphone.com";

    private int m_stunInterval = 60; // seconds

    private String m_publicAddress;

    private int m_publicPort = 5060;

    private int m_startRtpPort = 30000;

    private int m_stopRtpPort = 31000;

    private Location m_location;

    public void setUseStun(boolean useStun) {
        m_useStun = useStun;
    }

    public boolean isUseStun() {
        return m_useStun;
    }

    public String getPublicAddress() {
        return m_publicAddress;
    }

    public void setPublicAddress(String publicAddress) {
        m_publicAddress = publicAddress;
    }

    public int getPublicPort() {
        return m_publicPort;
    }

    public void setPublicPort(int publicPort) {
        m_publicPort = publicPort;
    }

    public int getStartRtpPort() {
        return m_startRtpPort;
    }

    public void setStartRtpPort(int startRtpPort) {
        m_startRtpPort = startRtpPort;
    }

    public int getStopRtpPort() {
        return m_stopRtpPort;
    }

    public void setStopRtpPort(int stopRtpPort) {
        m_stopRtpPort = stopRtpPort;
    }

    public String getStunAddress() {
        return m_stunAddress;
    }

    public void setStunAddress(String stunAddress) {
        m_stunAddress = stunAddress;
    }

    public int getStunInterval() {
        return m_stunInterval;
    }

    public void setStunInterval(int stunInterval) {
        m_stunInterval = stunInterval;
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }

}
