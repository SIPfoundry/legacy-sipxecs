/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in grandstream models
 */
public final class GrandstreamModel extends PhoneModel {

    private boolean m_isHandyTone;
    private boolean m_isFxsGxw;
    private boolean m_speedDial;
    private int m_speedDialKeys;
    private boolean m_speedDialExpansion;

    public GrandstreamModel() {
    }

    public void setIsHandyTone(boolean isHandyTone) {
        m_isHandyTone = isHandyTone;
    }

    public boolean isHandyTone() {
        return m_isHandyTone;
    }

    public void setIsFxsGxw(boolean isFxsGxw) {
        m_isFxsGxw = m_isFxsGxw;
    }

    public boolean isFxsGxw() {
        return m_isFxsGxw;
    }

    public boolean speedDial() {
        return m_speedDial;
    }

    public void setspeedDial(boolean speedDial) {
        m_speedDial = speedDial;
    }

    public int speedDialKeys() {
        return m_speedDialKeys;
    }

    public void setspeedDialKeys(int speedDialKeys) {
        m_speedDialKeys = speedDialKeys;
    }

    public boolean speedDialExpansion() {
        return m_speedDialExpansion;
    }

    public void setspeedDialExpansion(boolean speedDialExpansion) {
        m_speedDialExpansion = speedDialExpansion;
    }
}
