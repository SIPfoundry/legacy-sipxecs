/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Calendar;
import java.util.Date;

import org.hibernate.type.TimestampType;

/**
 * Standard Hibernate assume that type is kept in local time zone and will normalize it to UTC.
 * Use this type if your type is already normalized to UTC.
 */
public class UtcTimestampType extends TimestampType {
    private Calendar m_local;

    private Calendar getLocalCalendar() {
        if (m_local == null) {
            m_local = Calendar.getInstance();
        }
        return m_local;
    }

    /**
     * value has been already converted to what Java thought was UTC value, we need to revert the
     * results of that conversion
     */
    public void set(PreparedStatement st, Object value, int index) throws SQLException {
        Date date = (Date) value;
        Calendar local = getLocalCalendar();
        local.setTime(date);
        int offset = local.get(Calendar.ZONE_OFFSET);
        local.add(Calendar.MILLISECOND, -offset);
        Date localTime = local.getTime();
        super.set(st, localTime, index);
    }

    /**
     * result of this function will be converted to local value, we need to add now offset that
     * will be removed by that conversion
     */
    public Object get(ResultSet rs, String name) throws SQLException {
        Date value = (Date) super.get(rs, name);
        Calendar local = getLocalCalendar();
        local.setTime(value);
        int offset = local.get(Calendar.ZONE_OFFSET);
        local.add(Calendar.MILLISECOND, offset);
        return local.getTime();
    }
}
