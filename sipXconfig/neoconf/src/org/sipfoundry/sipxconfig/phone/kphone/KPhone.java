/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.kphone;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class KPhone extends Phone {
    private static final String REG_URI = "Registration/SipUri";
    private static final String REG_USER = "Registration/UserName";
    private static final String REG_SERVER = "Registration/SipServer";

    public KPhone() {
    }

    public String getProfileFilename() {
        return getSerialNumber() + ".kphonerc";
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new KPhoneLineDefaults(line));
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        int port = SipUri
                .parsePort(lineInfo.getRegistrationServerPort(), SipUri.DEFAULT_SIP_PORT);
        String uri = SipUri.formatIgnoreDefaultPort(lineInfo.getDisplayName(), lineInfo
                .getUserId(), lineInfo.getRegistrationServer(), port);

        line.setSettingValue(REG_URI, uri);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(REG_USER));
        String uri = line.getSettingValue(REG_URI);
        lineInfo.setDisplayName(SipUri.extractFullUser(uri));
        // TODO Extract server and port
        // lineInfo.setRegistrationServer(SipUri.extractServer(uri));
        return lineInfo;
    }

    public static class KPhoneLineDefaults {
        private Line m_line;

        public KPhoneLineDefaults(Line line) {
            m_line = line;
        }

        @SettingEntry(path = REG_URI)
        public String getUri() {
            return m_line.getUri();
        }

        @SettingEntry(path = REG_USER)
        public String getUserName() {
            String userName = null;
            User user = m_line.getUser();
            if (user != null) {
                userName = user.getUserName();
            }

            return userName;
        }

        @SettingEntry(path = REG_SERVER)
        public String getRegistrationServer() {
            return m_line.getPhoneContext().getPhoneDefaults().getDomainName();
        }
    }
}
