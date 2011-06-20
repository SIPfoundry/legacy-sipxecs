/*
 *
 *
 * Copyright (C) 2004-2009 iscoord ltd.
 * Beustweg 12, 8032 Zurich, Switzerland
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.isphone;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Setting;

public class IsphonePhone extends Phone {
    public static final String BEAN_ID = "isphone";

    public IsphonePhone() {
    }

    @Override
    public void initialize() {
        IsphoneDefaults defaults = new IsphoneDefaults(getPhoneContext().getPhoneDefaults(), this);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        IsphoneLineDefaults defaults = new IsphoneLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected ProfileContext<IsphonePhone> createContext() {
        return new IsphoneProfileContext(this, getModel().getProfileTemplate());
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(IsphoneConstants.DISPLAY_NAME, externalLine.getDisplayName());
        line.setSettingValue(IsphoneConstants.SIP_ID, externalLine.getUserId());
        line.setSettingValue(IsphoneConstants.AUTHENTICATION_PASSWORD, externalLine.getPassword());
        line.setSettingValue(IsphoneConstants.PROXY, externalLine.getRegistrationServer());
        line.setSettingValue(IsphoneConstants.VOICE_MAIL_ACCESS_CODE, externalLine.getVoiceMail());
        line.setSettingValue(IsphoneConstants.DOMAIN, externalLine.getRegistrationServer());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(IsphoneConstants.SIP_ID));
        lineInfo.setDisplayName(line.getSettingValue(IsphoneConstants.DISPLAY_NAME));
        lineInfo.setPassword(line.getSettingValue(IsphoneConstants.AUTHENTICATION_PASSWORD));
        lineInfo.setRegistrationServer(line.getSettingValue(IsphoneConstants.PROXY));
        lineInfo.setVoiceMail(line.getSettingValue(IsphoneConstants.VOICE_MAIL_ACCESS_CODE));
        return lineInfo;
    }

    @Override
    public String getProfileFilename() {
        StringBuilder buffer = new StringBuilder(getSerialNumber().toUpperCase());
        buffer.append(".xml");
        return buffer.toString();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    public Collection<Setting> getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        List<Setting> linesSettings = new ArrayList<Setting>(getMaxLineCount());

        Collection<Line> lines = getLines();
        int i = 0;
        Iterator<Line> ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(ilines.next().getSettings());
        }
        return linesSettings;
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
