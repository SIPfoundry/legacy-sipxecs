/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;

@SuppressWarnings("serial")
public class AlarmEvent implements Serializable {
    private static final DateFormat LOG_DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");
    private AlarmDefinition m_alarmDefinition;
    private Date m_date;
    private String m_message;

    /**
     * Parse a single line of log line into Alarm event
     *
     * @param logLine String in following format:
     *        "2009-04-29T10:39:30.778775Z":1:ALARM:WARNING:sipx
     *        .example.org:sipXsupervisor::SPX00002:"Some description."
     */
    protected AlarmEvent() {
    }

    public AlarmEvent(Date when, AlarmDefinition def, String message) {
        m_date = when;
        m_alarmDefinition = def;
        m_message = message;
    }

    public static AlarmEvent parseEvent(AlarmServerManager manager, String logLine) {
        // limit number of fields to 10 - last field 'description' can contain colons
        String[] tokens = StringUtils.split(logLine, ":", 10);
        AlarmEvent e = new AlarmEvent();
        e.m_alarmDefinition = manager.getAlarmDefinitions().get(tokens[8]);
        e.m_date = extractDate(logLine);
        return e;
    }

    private static Date extractDate(String line) {
        try {
            String token = StringUtils.substring(line, 1, 20);
            // alarm times are the UTC times from the alarms log
            LOG_DATE_FORMAT.setTimeZone(TimeZone.getTimeZone("UTC"));
            return LOG_DATE_FORMAT.parse(token);
        } catch (ParseException ex) {
            return new Date(0);
        }
    }

    public Date getDate() {
        return m_date;
    }

    public AlarmDefinition geAlarmDefinition() {
        return m_alarmDefinition;
    }

    public AlarmDefinition getAlarmDefinition() {
        return m_alarmDefinition;
    }

    public String getMessage() {
        return m_message;
    }
}
