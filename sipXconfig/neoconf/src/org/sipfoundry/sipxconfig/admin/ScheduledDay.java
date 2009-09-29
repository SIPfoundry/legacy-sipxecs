/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Calendar;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public final class ScheduledDay extends Enum {
    public static final ScheduledDay WEEKEND = new ScheduledDay("Weekend", -2);
    public static final ScheduledDay WEEKDAYS = new ScheduledDay("Weekdays", -1);
    public static final ScheduledDay EVERYDAY = new ScheduledDay("Every day");
    public static final ScheduledDay SUNDAY = new ScheduledDay("Sunday", Calendar.SUNDAY);
    public static final ScheduledDay MONDAY = new ScheduledDay("Monday", Calendar.MONDAY);
    public static final ScheduledDay TUESDAY = new ScheduledDay("Tuesday", Calendar.TUESDAY);
    public static final ScheduledDay WEDNESDAY = new ScheduledDay("Wednesday", Calendar.WEDNESDAY);
    public static final ScheduledDay THURSDAY = new ScheduledDay("Thursday", Calendar.THURSDAY);
    public static final ScheduledDay FRIDAY = new ScheduledDay("Friday", Calendar.FRIDAY);
    public static final ScheduledDay SATURDAY = new ScheduledDay("Saturday", Calendar.SATURDAY);

    public static final ScheduledDay[] DAYS_OF_WEEK = {
        SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY,
    };

    public static final ScheduledDay[] DAYS_OF_WEEK_PLUS_EVERYDAY = {
        EVERYDAY, SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY,
    };

    public static final ScheduledDay[] WEEKEND_DAYS = {
        SATURDAY, SUNDAY,
    };

    public static final ScheduledDay[] WEEK_DAYS = {
        MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY,
    };

    private int m_dayOfWeek;

    private ScheduledDay(String id) {
        super(id);
    }

    private ScheduledDay(String id, int dayOfWeek) {
        super(id);
        m_dayOfWeek = dayOfWeek;
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(ScheduledDay.class);
        }
    }

    /** Map the day ID string to a ScheduledDay object */
    public static ScheduledDay getScheduledDay(String id) {
        return (ScheduledDay) getEnum(ScheduledDay.class, id);
    }

    public static ScheduledDay getScheduledDay(int calendarDayOfWeek) {
        if (calendarDayOfWeek == -2) {
            return ScheduledDay.WEEKEND;
        }
        if (calendarDayOfWeek == -1) {
            return ScheduledDay.WEEKDAYS;
        }
        if (calendarDayOfWeek == 0) {
            return ScheduledDay.EVERYDAY;
        }
        if (calendarDayOfWeek < 1 || calendarDayOfWeek > DAYS_OF_WEEK.length) {
            throw new IllegalArgumentException("Pass Calendar.MONDAY, Calendar.TUESDAY etc.");
        }
        return DAYS_OF_WEEK[calendarDayOfWeek - 1];
    }

    /**
     * @return example Calendar.SATURDAY
     */
    public int getDayOfWeek() {
        return m_dayOfWeek;
    }

}
