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

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

public class GrandstreamProfileWriter extends AbstractSettingVisitor {
    protected static final char LF = 0x0a;
    private OutputStream m_wtr;
    private final GrandstreamPhone m_phone;
    private int m_lineIndex;
    private final Set m_ignore = new HashSet();

    GrandstreamProfileWriter(GrandstreamPhone phone) {
        m_phone = phone;
    }

    public void write(OutputStream wtr) {
        setOutputStream(wtr);
        write();
    }

    protected void write() {
        m_phone.getSettings().acceptVisitor(this);
        for (Line line : getLines()) {
            line.getSettings().acceptVisitor(this);
            m_lineIndex++;
        }
    }

    protected void setOutputStream(OutputStream wtr) {
        m_wtr = wtr;
    }

    protected void writeString(String line) {
        try {
            m_wtr.write(line.getBytes("UTF-8"));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    protected GrandstreamPhone getPhone() {
        return m_phone;
    }

    @Override
    public void visitSetting(Setting setting) {
        writeProfileEntry(setting.getProfileName(), setting.getValue());
    }

    String nonNull(String value) {
        return value == null ? StringUtils.EMPTY : value;
    }

    void writeProfileEntry(String name, String value) {
        if (m_ignore.contains(name)) {
            return;
        }
        if (isCompositeIpAddress(name)) {
            writeIpAddress(name, value);
        } else {
            writeLine(name, value);
        }
    }

    void writeLine(String name, String value) {
        String lname = name;
        if (isCompositeProfileName(lname)) {
            String[] names = name.split("-");
            lname = names[m_lineIndex];
        }
        writeLineEntry(lname, value);
    }

    protected void writeLineEntry(String name, String value) {
        if (value != null) {
            String line = name + " = " + nonNull(value) + LF;
            writeString(line);
        }
    }

    boolean isCompositeProfileName(String name) {
        return name.indexOf('-') >= 0;
    }

    boolean isCompositeIpAddress(String name) {
        return name.indexOf(',') >= 0;
    }

    void writeIpAddress(String name, String value) {
        String[] names = name.split(",");
        String[] values = StringUtils.defaultString(value).split("\\.");
        for (int i = 0; i < names.length; i++) {
            String svalue = i < values.length ? values[i] : StringUtils.EMPTY;
            writeLine(names[i], svalue);
        }
    }

    public Collection<Line> getLines() {
        int lineCount = getPhone().getModel().getMaxLineCount();
        Collection<Line> lines = new ArrayList(lineCount);
        if (getPhone().getLines().isEmpty()) {
            Line line = getPhone().createSpecialPhoneProvisionUserLine();
            line.setSettingValue("port/P270-P417-P517-P617-P1717-P1817", line.getUser().getDisplayName());
            lines.add(line);
        } else {
            lines.addAll(getPhone().getLines());
            // copy in blank lines of all unused lines
            for (int i = lines.size(); i < lineCount; i++) {
                Line line = getPhone().createLine();
                lines.add(line);
            }
        }

        return lines;
    }

}
