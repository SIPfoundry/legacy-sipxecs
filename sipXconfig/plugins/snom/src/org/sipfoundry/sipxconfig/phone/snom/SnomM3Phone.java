/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.snom;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class SnomM3Phone extends Phone {
    public static final String BEAN_ID = "snomM3Phone";

    public SnomM3Phone() {
    }

    @Override
    public void initialize() {
        SnomM3Defaults defaults = new SnomM3Defaults(getPhoneContext().getPhoneDefaults(), this);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {

        SnomM3LineDefaults defaults = new SnomM3LineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected ProfileContext<SnomM3Phone> createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Collection<PhonebookEntry> phoneBook = getPhoneContext().getPhonebookEntries(this);
        return new SnomM3ProfileContext(this, speedDial, phoneBook, getModel().getProfileTemplate());
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(SnomM3Constants.DISPLAY_NAME, externalLine.getDisplayName());
        line.setSettingValue(SnomM3Constants.USER_NAME, externalLine.getUserId());
        line.setSettingValue(SnomM3Constants.PASSWORD, externalLine.getPassword());
        line.setSettingValue(SnomM3Constants.OUTBOUND_PROXY, externalLine.getRegistrationServer());
        //line.setSettingValue(SnomConstants.MAILBOX, externalLine.getVoiceMail());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(SnomM3Constants.USER_NAME));
        lineInfo.setDisplayName(line.getSettingValue(SnomM3Constants.DISPLAY_NAME));
        lineInfo.setPassword(line.getSettingValue(SnomM3Constants.PASSWORD));
        lineInfo.setRegistrationServer(line.getSettingValue(SnomM3Constants.OUTBOUND_PROXY));
        //lineInfo.setVoiceMail(line.getSettingValue(SnomConstants.MAILBOX));
        return lineInfo;
    }

    @Override
    public String getProfileFilename() {
        StringBuilder buffer = new StringBuilder(getSerialNumber().toLowerCase());
        buffer.append(".cfg");
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

        for (; i < lineCount; i++) {
            Line line = createLine();
            line.setPhone(this);
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
