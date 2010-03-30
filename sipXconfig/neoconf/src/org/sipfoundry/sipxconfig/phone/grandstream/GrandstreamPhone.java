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
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    private static final String APPLICATION_OCTET_STREAM = "application/octet-stream";
    private static final String TIMEZONE_SETTING = "phone/P64";
    private static final String DAYLIGHT_SETTING = "phone/P75";
    private static final String GXW_TIMEZONE_SETTING = "gateway/P246";
    private static final String USERID_PATH = "port/P35-P404-P504-P604-P1704-P1804";
    private static final String HT_USERID_PATH = "port/P35-P735";
    private static final String GXW_USERID_PATH = "port/P4060-P4061-P4062-P4063-P4064-P4065-P4066-P4067";
    private static final String AUTHID_PATH = "port/P36-P405-P505-P605-P1705-P1805";
    private static final String HT_AUTHID_PATH = "port/P36-P736";
    private static final String GXW_AUTHID_PATH = "port/P4090-P4091-P4092-P4093-P4094-P4095-P4096-P4097";
    private static final String PASSWORD_PATH = "port/P34-P406-P506-P606-P1706-P1806";
    private static final String HT_PASSWORD_PATH = "port/P34-P734";
    private static final String GXW_PASSWORD_PATH = "port/P4120-P4121-P4122-P4123-P4124-P4125-P4126-P4127";
    private static final String DISPLAY_NAME_PATH = "port/P3-P407-P507-P607-P1707-P1807";
    private static final String HT_DISPLAY_NAME_PATH = "port/P3-P703";
    private static final String GXW_DISPLAY_NAME_PATH = "port/P4180-P4181-P4182-P4183-P4184-P4185-P4186-P4187";
    private static final String REGISTRATION_SERVER_PATH = "port/P47-P402-P502-P602-P1702-P1802";
    private static final String LINE_ACTIVE_PATH = "port/P271-P401-P501-P601-P1701-P1801";
    private static final String OUTBOUND_PROXY_PATH = "port/P48-P403-P503-P603-P1703-P1803";
    private static final String HT_REGISTRATION_SERVER_PATH = "port/P47-P747";
    private static final String HT_OUTBOUND_PROXY_PATH = "port/P48-P748";
    private static final String GXW_REGISTRATION_SERVER_PATH = "account-proxy/P47";
    private static final String GXW_OUTBOUND_PROXY_PATH = "account-proxy/P48";
    private static final String GXW_HUNTGROUP_PATH = "port/P4300-P4301-P4302-P4303-P4304-P4305-P4306-P4307";
    private static final String VOICEMAIL_PATH = "port/P33-P426-P526-P626-P1726-P1826";
    private static final String DOT = ".";

    private boolean m_isTextFormatEnabled;

    public GrandstreamPhone() {
    }

    @Override
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        SettingExpressionEvaluator evaluator = new GrandstreamSettingExpressionEvaluator(
                getModel().getModelId());
        return evaluator;
    }

    @Override
    public void initialize() {
        GrandstreamDefaults defaults = new GrandstreamDefaults(getPhoneContext()
                .getPhoneDefaults());
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
        } else if (getGsModel().isFxsGxw()) {
            lineInfo.setDisplayName(line.getSettingValue(GXW_DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(GXW_USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(GXW_PASSWORD_PATH));
            lineInfo.setHuntgroup(line.getSettingValue(GXW_HUNTGROUP_PATH));
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
            line.setSettingValue(HT_OUTBOUND_PROXY_PATH, lineInfo.getRegistrationServer());
        } else if (getGsModel().isFxsGxw()) {
            line.setSettingValue(GXW_DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(GXW_USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(GXW_PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(GXW_HUNTGROUP_PATH, lineInfo.getHuntgroup());
        } else {
            line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(REGISTRATION_SERVER_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(OUTBOUND_PROXY_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(VOICEMAIL_PATH, lineInfo.getVoiceMail());
        }
    }

    public GrandstreamModel getGsModel() {
        return (GrandstreamModel) getModel();
    }

    public int getLineNumber(int lineId) {
        int lineOffset = 0;
        Iterator ilines = getLines().iterator();
        for (; ilines.hasNext(); lineOffset++) {
            if (((Line) ilines.next()).getId() == lineId) {
                return lineOffset + 1;
            }
        }

        return 0;
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

    @Override
    public String getModelLabel() {
        return getModel().getLabel();
    }

    @Override
    public String getProfileFilename() {
        String phoneFilename = getSerialNumber();
        return "cfg" + phoneFilename.toLowerCase();
    }


    @Override
    public Profile[] getProfileTypes() {
        String profileFilename = getProfileFilename();
        Profile profile = new Profile(profileFilename, APPLICATION_OCTET_STREAM);
        return new Profile[] {
            profile
        };
    }

    public void setDefaultIfExists(Setting sroot, String param, String value) {
        if (sroot.getSetting(param) != null) {
            sroot.getSetting(param).setValue(value);
        }
    }

    public static class GrandstreamDefaults {
        private final DeviceDefaults m_defaults;

        GrandstreamDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(paths = { "upgrade/P192", "upgrade/P237" })
        public String getTftpServer() {
            return m_defaults.getTftpServer();
        }

        private String zoneOffset(String zone, int offset) {
            String zoneOffset;
            int offsetHours = offset / 60;
            int offsetMinutes = offset % 60;
            // Display "-" for a postive offset and "+" for a negative or zero offset
            if (offset > 0) {
                zoneOffset = (zone + "-" + Integer.toString(offsetHours));
            } else {
                zoneOffset = (zone + "%2b" + Integer.toString(-offsetHours));
            }
            if ((offsetMinutes) > 0) {
                zoneOffset = zoneOffset + ":" + Integer.toString(offsetMinutes);
            }
            return zoneOffset;
        }

        private String zoneDstDate(int dstMonth, int dstWeek, int dstDay) {
            String month = Integer.toString(dstMonth + 1);
            String week = Integer.toString(dstWeek);
            String day = Integer.toString(dstDay - 1);
            return (",M" + month + DOT + week + DOT + day);
        }

        private String zoneCustomTime(int standardOffset, int daylightOffset) {
            DeviceTimeZone zone = m_defaults.getTimeZone();
            String timezone = zoneOffset("MTZ", standardOffset)
                + zoneOffset("MDT", daylightOffset)
                + zoneDstDate(zone.getStartMonth(), zone.getStartWeek(), zone.getStartDayOfWeek())
                + zoneDstDate(zone.getStopMonth(), zone.getStopWeek(), zone.getStopDayOfWeek());
            return timezone;
        }

        @SettingEntry(path = TIMEZONE_SETTING)
        public int getTimeOffset() {
            // Get the offset in minutes where GMT=720.
            int offset = ((m_defaults.getTimeZone().getOffset()) + (12 * 60));
            return offset;
        }

        @SettingEntry(path = DAYLIGHT_SETTING)
        public boolean getUseDaylight() {
            return m_defaults.getTimeZone().getUseDaylight();
        }

        @SettingEntry(path = GXW_TIMEZONE_SETTING)
        public String getGatewayTimeOffset() {
            int offsetInDst;
            if (m_defaults.getTimeZone().getUseDaylight()) {
                offsetInDst = m_defaults.getTimeZone().getOffsetWithDst();
            } else {
                offsetInDst = m_defaults.getTimeZone().getOffset();
            }
            String timezone = zoneCustomTime(m_defaults.getTimeZone().getOffset(), offsetInDst);
            return timezone;
        }

        @SettingEntry(path = "network/P30")
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(paths = { GXW_REGISTRATION_SERVER_PATH, GXW_OUTBOUND_PROXY_PATH })
        public String getRegistationServer() {
            return m_defaults.getDomainName();
        }
    }

    public static class GrandstreamLineDefaults {
        private final GrandstreamPhone m_phone;
        private final Line m_line;

        GrandstreamLineDefaults(GrandstreamPhone phone, Line line) {
            m_phone = phone;
            m_line = line;
        }

        @SettingEntry(paths = { USERID_PATH, HT_USERID_PATH, GXW_USERID_PATH,
                AUTHID_PATH, HT_AUTHID_PATH, GXW_AUTHID_PATH })
        public String getUserId() {
            String userId = null;
            User u = m_line.getUser();
            if (u != null) {
                userId = u.getUserName();
            }

            return userId;
        }

        @SettingEntry(paths = { PASSWORD_PATH, HT_PASSWORD_PATH, GXW_PASSWORD_PATH })
        public String getPassword() {
            String password = null;
            User u = m_line.getUser();
            if (u != null) {
                password = u.getSipPassword();
            }

            return password;
        }

        @SettingEntry(paths = { DISPLAY_NAME_PATH, HT_DISPLAY_NAME_PATH, GXW_DISPLAY_NAME_PATH })
        public String getDisplayName() {
            String displayName = null;
            User u = m_line.getUser();
            if (u != null) {
                displayName = u.getDisplayName();
            }

            return displayName;
        }

        @SettingEntry(paths = { REGISTRATION_SERVER_PATH, OUTBOUND_PROXY_PATH, HT_REGISTRATION_SERVER_PATH,
                HT_OUTBOUND_PROXY_PATH })
        public String getRegistationServer() {
            return m_phone.getPhoneContext().getPhoneDefaults().getDomainName();
        }

        @SettingEntry(path = GXW_HUNTGROUP_PATH)
        public String getHuntgroup() {
            // Use the line number: line1="1", line2="2", etc.
            int lineNumber = m_phone.getLineNumber(m_line.getId());
            return Integer.toString(lineNumber);
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
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    @Override
    protected ProfileContext createContext() {
        return new GrandstreamProfileContext(this, m_isTextFormatEnabled);
    }

    @Override
    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("&phone.line.not.valid");
        }

        Line line = getLines().get(0);
        LineInfo info = line.getLineInfo();
        String password = info.getPassword();
        try {
            byte[] resetPayload = new ResetPacket(password, getSerialNumber()).getResetMessage();
            getSipService().sendNotify(line.getAddrSpec(), "sys-control", APPLICATION_OCTET_STREAM, resetPayload);
        } catch (IllegalArgumentException iae) {
            throw new RestartException("&phone.reset.packet");
        } catch (RuntimeException re) {
            throw new RestartException("&phone.sip.exception");
        }
    }

    static class GrandstreamSettingExpressionEvaluator implements SettingExpressionEvaluator {
        private final String m_model;

        public GrandstreamSettingExpressionEvaluator(String model) {
            m_model = model;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_model.matches(expression);
        }
    }
}
