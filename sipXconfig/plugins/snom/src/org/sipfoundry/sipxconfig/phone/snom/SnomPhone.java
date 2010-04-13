/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
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
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.beans.factory.annotation.Required;

public class SnomPhone extends Phone {
    public static final String BEAN_ID = "snom";

    private SpeedDialManager m_speedDialManager;

    public SnomPhone() {
    }

    @Required
    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    @Override
    public void initialize() {
        SnomDefaults defaults = new SnomDefaults(getPhoneContext().getPhoneDefaults(), this);
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        SpeedDial userSpeedDial = null;
        if (line.getUser() != null) {
            userSpeedDial = m_speedDialManager.getSpeedDialForUserId(line.getUser().getId(), false);
        }

        SnomLineDefaults defaults = new SnomLineDefaults(getPhoneContext().getPhoneDefaults(), line, userSpeedDial);
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected ProfileContext<SnomPhone> createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        Collection<PhonebookEntry> phoneBook = getPhoneContext().getPhonebookEntries(this);
        return new SnomProfileContext(this, speedDial, phoneBook, getModel().getProfileTemplate());
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(SnomConstants.DISPLAY_NAME, externalLine.getDisplayName());
        line.setSettingValue(SnomConstants.USER_NAME, externalLine.getUserId());
        line.setSettingValue(SnomConstants.PASSWORD, externalLine.getPassword());
        line.setSettingValue(SnomConstants.USER_HOST, externalLine.getRegistrationServer());
        line.setSettingValue(SnomConstants.MAILBOX, externalLine.getVoiceMail());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(SnomConstants.USER_NAME));
        lineInfo.setDisplayName(line.getSettingValue(SnomConstants.DISPLAY_NAME));
        lineInfo.setPassword(line.getSettingValue(SnomConstants.PASSWORD));
        lineInfo.setRegistrationServer(line.getSettingValue(SnomConstants.USER_HOST));
        lineInfo.setVoiceMail(line.getSettingValue(SnomConstants.MAILBOX));
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
