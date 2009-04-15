/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Nortel phone.
 */
public class NortelPhone extends Phone {

    private static final String NORTEL_FORCE_CONFIG = "nortel/11xxeSIP.cfg";

    public NortelPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        NortelPhoneDefaults defaults = new NortelPhoneDefaults(phoneDefaults, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        DeviceDefaults phoneDefaults = getPhoneContext().getPhoneDefaults();
        Line line = new Line();
        NortelPhoneDefaults defaults = new NortelPhoneDefaults(phoneDefaults, line);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber().toUpperCase() + ".cfg";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = NortelPhoneDefaults.getLineInfo(line);
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {

        NortelPhoneDefaults.setLineInfo(line, lineInfo);
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    protected void copyFiles(ProfileLocation location) {
        super.copyFiles(location);
        getProfileGenerator().copy(location, NORTEL_FORCE_CONFIG, "1120eSIP.cfg");
        getProfileGenerator().copy(location, NORTEL_FORCE_CONFIG, "1140eSIP.cfg");
    }

    public static class NortelPhoneDefaults {

        private static final String NETWORK_DNS_DOMAIN = "NETWORK/DNS_DOMAIN";
        private static final String VOIP_SIP_DOMAIN1 = "VOIP/SIP_DOMAIN1";
        private static final String VOIP_SERVER_IP1_1 = "VOIP/SERVER_IP1_1";
        private static final String VOIP_SERVER_PORT1_1 = "VOIP/SERVER_PORT1_1";
        private static final String VOIP_SERVER_IP1_2 = "VOIP/SERVER_IP1_2";
        private static final String VOIP_DEF_USER1 = "VOIP/DEF_USER1";
        private static final String VM_VMAIL = "VM/VMAIL";

        private Line m_line;
        private DeviceDefaults m_defaults;

        NortelPhoneDefaults(DeviceDefaults defaults, Line line) {
            m_line = line;
            m_defaults = defaults;
        }

        @SettingEntry(paths = { NETWORK_DNS_DOMAIN, VOIP_SIP_DOMAIN1 })
        public String getSipDomain1() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(paths = { VOIP_SERVER_IP1_1, VOIP_SERVER_IP1_2 })
        public String getServerIp() {
            return m_defaults.getProxyServerAddr();
        }

        @SettingEntry(path = VOIP_SERVER_PORT1_1)
        public String getProxyPort() {
            return m_defaults.getProxyServerSipPort();
        }

        @SettingEntry(path = VOIP_DEF_USER1)
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }
            return userName;
        }

        @SettingEntry(path = VM_VMAIL)
        public String getVoicemailExtension() {
            return m_defaults.getVoiceMail();
        }

        public static LineInfo getLineInfo(Line line) {
            LineInfo lineInfo = new LineInfo();
            lineInfo.setUserId(line.getSettingValue(VOIP_DEF_USER1));
            lineInfo.setRegistrationServer(line.getSettingValue(VOIP_SERVER_IP1_1));
            lineInfo.setRegistrationServerPort(line.getSettingValue(VOIP_SERVER_PORT1_1));
            return lineInfo;
        }

        public static void setLineInfo(Line line, LineInfo lineInfo) {
            line.setSettingValue(VOIP_DEF_USER1, lineInfo.getUserId());
            line.setSettingValue(VOIP_SERVER_IP1_1, lineInfo.getRegistrationServer());
            line.setSettingValue(VOIP_SERVER_PORT1_1, lineInfo.getRegistrationServerPort());
        }
    }
}
