/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.login;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.lang.StringUtils;

public class LoginEvent implements Serializable {

    public static final String SUCCESS = "SUCCESS";
    public static final String FAILURE = "FAILED";

    private static final String LOGIN_TOKEN = "LOGIN";
    private static final String IP_TOKEN = "remote IP";
    private static final String USER_TOKEN = "user";

    private static final String LOG_FORMAT = "%s | LOGIN %s | remote IP %s | user %s ";
    private static final String WRITE_LOG_FORMAT = "%s %s, %s %s, %s %s";
    private static final DateFormat LOG_DATE_FORMAT = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    private String m_logEntry;
    private Date m_date;
    private String m_type;
    private String m_ip;
    private String m_user;

    public LoginEvent(String line) {
        m_logEntry = line;
        m_date = extractDate(line);
        m_type = StringUtils.trim(extractTokenValue(m_logEntry, LOGIN_TOKEN));
        m_ip = StringUtils.trim(extractTokenValue(m_logEntry, IP_TOKEN));
        m_user = StringUtils.trim(extractTokenValue(m_logEntry, USER_TOKEN));
    }

    public Date getDateValue() {
        return m_date;
    }

    public String getIpValue() {
        return m_ip;
    }

    public String getType() {
        return m_type;
    }

    public String getUserValue() {
        return m_user;
    }

    public String formatLogEntry() {
        return String.format(LOG_FORMAT, LOG_DATE_FORMAT.format(m_date), m_type, m_ip, m_user);
    }

    public static String formatLogToRecord(String... logValues) {
        return String.format(WRITE_LOG_FORMAT, LOGIN_TOKEN, logValues[0], USER_TOKEN,
                logValues[1], IP_TOKEN, logValues[2]);
    }

    Date extractDate(String line) {
        Date date = null;
        try {
            String token = StringUtils.substringBetween(line, "\"");
            date = LOG_DATE_FORMAT.parse(token);
        } catch (ParseException ex) {
            return null;
        }
        return date;
    }

    String extractTokenValue(String line, String token) {
        String separator = ",";
        if (StringUtils.substringBetween(line, token, separator) == null) {
            return StringUtils.substringAfterLast(line, token);
        }
        return StringUtils.substringBetween(line, token, separator);
    }

}
