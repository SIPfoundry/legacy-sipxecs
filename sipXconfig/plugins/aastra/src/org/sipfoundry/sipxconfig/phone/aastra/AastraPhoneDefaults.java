/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AastraPhoneDefaults {
    private DeviceDefaults m_defaults;

    AastraPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    @SettingEntry(path = "td/dstconf")
    public int getDSTSavings() {
        int time = 0;
        // FIXME: not sure why it's 30 or 60
        return time;
    }

    @SettingEntry(path = "td/timeZoneName")
    public long getGmtOffset() {
        return getZone().getOffset();
    }

    @SettingEntry(path = "sipNet/sipRegistarIp")
    public String getRegistrationServer() {
        return m_defaults.getDomainName();
    }

    @SettingEntry(path = "dialPlan/emergencyDialPlan")
    public String getEmergencyNumber() {
        return m_defaults.getEmergencyNumber();
    }

}
