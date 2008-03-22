package org.sipfoundry.sipxconfig.phone.aastra;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class AastraPhoneDefaults {
    private DeviceDefaults m_defaults;
    private SpeedDial m_speedDial;

    AastraPhoneDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    private DeviceTimeZone getZone() {
        return m_defaults.getTimeZone();
    }

    // FIXME: not sure why it's 30 or 60
    @SettingEntry(path = "td/dstconf")
    public int getDSTSavings() {
	int time = 0;
	if(m_defaults.getDSTSavings() == 1) {
	    time = 30;
	} else if(m_defaults.getDSTSavings() == 2) {
	    time = 60;
	}
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
