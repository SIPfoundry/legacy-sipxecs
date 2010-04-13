/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.HashCodeBuilder;

public class AlarmEvent implements Serializable {
    private static final DateFormat LOG_DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss");

    private final Alarm m_alarm;

    private final Date m_date;

    /**
     * Parse a single line of log line into Alarm event
     *
     * @param logLine String in following format:
     *        "2009-04-29T10:39:30.778775Z":1:ALARM:WARNING:sipx
     *        .example.org:sipXsupervisor::SPX00002:"Some description."
     */
    public AlarmEvent(String logLine) {
        // limit number of fields to 10 - last field 'description' can contain colons
        String[] tokens = StringUtils.split(logLine, ":", 10);
        Alarm alarm = new Alarm();
        alarm.setCode(tokens[8]);
        alarm.setDescription(StringEscapeUtils.unescapeJava(tokens[9]));
        alarm.setSeverity(tokens[5]);

        m_alarm = alarm;
        m_date = extractDate(logLine);
    }

    private Date extractDate(String line) {
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

    public Alarm getAlarm() {
        return m_alarm;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_alarm).append(m_date).toHashCode();
    }
}
