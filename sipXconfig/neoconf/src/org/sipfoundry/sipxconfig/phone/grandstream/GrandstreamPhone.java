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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    public static final String BEAN_ID = "grandstream";

    private static final String TIMEZONE_SETTING = "phone/P64";
    private static final String USERID_PATH = "port/P35-P404-P504-P604";
    private static final String HT_USERID_PATH = "port/P35-P735";
    private static final String AUTHID_PATH = "port/P36-P405-P505-P605";
    private static final String HT_AUTHID_PATH = "port/P36-P736";
    private static final String PASSWORD_PATH = "port/P34-P406-P506-P606";
    private static final String HT_PASSWORD_PATH = "port/P34-P734";
    private static final String DISPLAY_NAME_PATH = "port/P3-P407-P507-P607";
    private static final String HT_DISPLAY_NAME_PATH = "port/P3-P703";
    private static final String REGISTRATION_SERVER_PATH = "port/P47-P402-P502-P602";
    private static final String LINE_ACTIVE_PATH = "port/P271-P401-P501-P601";
    // unclear what 2nd copy is
    private static final String REGISTRATION_SERVER2_PATH = "port/P48-P403-P503-P603";
    private static final String HT_REGISTRATION_SERVER_PATH = "port/P47-P747";
    // unclear what 2nd copy is
    private static final String HT_REGISTRATION_SERVER2_PATH = "port/P48-P748";
    private static final String VOICEMAIL_PATH = "port/P33-P426-P526-P626";

    private boolean m_isTextFormatEnabled;

    public GrandstreamPhone() {
        super(new GrandstreamModel());
        setPhoneTemplate("grandstream/grandstream.vm");
    }

    @Override
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        SettingExpressionEvaluator evaluator = new GrandstreamSettingExpressionEvaluator(getModel()
                .getModelId());
        return evaluator;
    }

    @Override
    public void initialize() {
        GrandstreamDefaults defaults = new GrandstreamDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        GrandstreamLineDefaults defaults = new GrandstreamLineDefaults(this, line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        if (getGsModel().isHandyTone()) {
            lineInfo.setDisplayName(line.getSettingValue(HT_DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(HT_USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(HT_PASSWORD_PATH));
            lineInfo.setRegistrationServer(line.getSettingValue(HT_REGISTRATION_SERVER_PATH));
        } else {
            lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_PATH));
            lineInfo.setVoiceMail(line.getSettingValue(VOICEMAIL_PATH));
        }
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        if (getGsModel().isHandyTone()) {
            line.setSettingValue(HT_DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(HT_USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(HT_PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(HT_REGISTRATION_SERVER_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(HT_REGISTRATION_SERVER2_PATH, lineInfo.getRegistrationServer());
        } else {
            line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(REGISTRATION_SERVER_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(REGISTRATION_SERVER2_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(VOICEMAIL_PATH, lineInfo.getVoiceMail());
        }
    }

    public GrandstreamModel getGsModel() {
        return (GrandstreamModel) getModel();
    }

    /**
     * Generate files in text format. Won't be usable by phone, but you can use grandstreams
     * config tool to convert manually. This is mostly for debugging
     * 
     * @param isTextFormatEnabled true to save as text, default is false
     */
    public void setTextFormatEnabled(boolean isTextFormatEnabled) {
        m_isTextFormatEnabled = isTextFormatEnabled;
    }

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return "cfg" + phoneFilename.toLowerCase();
    }

    public void setDefaultIfExists(Setting sroot, String param, String value) {
        if (sroot.getSetting(param) != null) {
            sroot.getSetting(param).setValue(value);
        }
    }

    public static class GrandstreamDefaults {
        private DeviceDefaults m_defaults;

        GrandstreamDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(paths = { "upgrade/__TFTPServer-213", "upgrade/__TFTPServerOld-41", "upgrade/P192",
                "upgrade/P237" })
        public String getTftpServer() {
            return m_defaults.getTftpServer();
        }

        @SettingEntry(path = TIMEZONE_SETTING)
        public int getTimeOffset() {
            int offset = ((m_defaults.getTimeZone().getOffsetWithDst() / 60) + (12 * 60));
            return offset;
        }
        
        @SettingEntry(path = "network/P30")
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }
    }

    public static class GrandstreamLineDefaults {
        private GrandstreamPhone m_phone;
        private Line m_line;

        GrandstreamLineDefaults(GrandstreamPhone phone, Line line) {
            m_phone = phone;
            m_line = line;
        }

        @SettingEntry(paths = { USERID_PATH, HT_USERID_PATH, AUTHID_PATH, HT_AUTHID_PATH })
        public String getUserId() {
            String userId = null;
            User u = m_line.getUser();
            if (u != null) {
                userId = u.getUserName();
            }

            return userId;
        }

        @SettingEntry(paths = { PASSWORD_PATH, HT_PASSWORD_PATH })
        public String getPassword() {
            String password = null;
            User u = m_line.getUser();
            if (u != null) {
                password = u.getSipPassword();
            }

            return password;
        }

        @SettingEntry(paths = { DISPLAY_NAME_PATH, HT_DISPLAY_NAME_PATH })
        public String getDisplayName() {
            String displayName = null;
            User u = m_line.getUser();
            if (u != null) {
                displayName = u.getDisplayName();
            }

            return displayName;
        }

        @SettingEntry(paths = { REGISTRATION_SERVER_PATH, REGISTRATION_SERVER2_PATH, 
                HT_REGISTRATION_SERVER_PATH, HT_REGISTRATION_SERVER2_PATH })
        public String getRegistationServer() {
            return m_phone.getPhoneContext().getPhoneDefaults().getDomainName();
        }

        @SettingEntry(path = VOICEMAIL_PATH)
        public String getVoicemail() {
            return m_phone.getPhoneContext().getPhoneDefaults().getVoiceMail();
        }

        @SettingEntry(path = LINE_ACTIVE_PATH)
        public boolean isLineActive() {
            boolean active = !StringUtils.isBlank(getUserId());
            return active;
        }
    }

    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPosition(i);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    protected ProfileContext createContext() {
        return new GrandstreamProfileContext(this, m_isTextFormatEnabled);
    }

    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        Line line = getLines().get(0);
        LineInfo info = line.getLineInfo();
        String password = info.getPassword();
        byte[] resetPayload = new ResetPacket(password, getSerialNumber()).getResetMessage();
        String event = "Content-Type: application/octet-stream\r\n" + "Event: sys-control\r\n";

        getSipService().sendNotify(line.getUri(), info.getRegistrationServer(),
                info.getRegistrationServerPort(), event, resetPayload);
    }

    static class GrandstreamSettingExpressionEvaluator implements SettingExpressionEvaluator {
        private String m_model;

        public GrandstreamSettingExpressionEvaluator(String model) {
            m_model = model;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_model.matches(expression);
        }
    }
}
