/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class ActiveConferenceMember implements PrimaryKeySource {
    private int m_id;
    private String m_name;
    private String m_uuid;
    private int m_volumeIn;
    private int m_volumeOut;
    private int m_energyLevel;
    private boolean m_canHear;
    private boolean m_canSpeak;

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public boolean getCanHear() {
        return m_canHear;
    }

    public void setCanHear(boolean canHear) {
        this.m_canHear = canHear;
    }

    public boolean getCanSpeak() {
        return m_canSpeak;
    }

    public void setCanSpeak(boolean canSpeak) {
        this.m_canSpeak = canSpeak;
    }

    public String getUuid() {
        return m_uuid;
    }

    public void setUuid(String uuid) {
        m_uuid = uuid;
    }

    public int getVolumeIn() {
        return m_volumeIn;
    }

    public void setVolumeIn(int volumeIn) {
        m_volumeIn = volumeIn;
    }

    public int getVolumeOut() {
        return m_volumeOut;
    }

    public void setVolumeOut(int volumeOut) {
        m_volumeOut = volumeOut;
    }

    public int getEnergyLevel() {
        return m_energyLevel;
    }

    public void setEnergyLevel(int energyLevel) {
        m_energyLevel = energyLevel;
    }

    public Object getPrimaryKey() {
        return Integer.valueOf(m_id);
    }
}
