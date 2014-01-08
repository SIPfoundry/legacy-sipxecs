/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.attendant;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.util.HolidayPeriod;

public class Holiday extends ScheduledAttendant {
    private List<HolidayPeriod> m_periods = new ArrayList<HolidayPeriod>();

    public void addPeriod(HolidayPeriod period) {
        m_periods.add(period);
    }

    public void removePeriod(HolidayPeriod period) {
        m_periods.remove(period);
    }

    public List<HolidayPeriod> getPeriods() {
        return m_periods;
    }

    public void setPeriods(List<HolidayPeriod> holidayPeriods) {
        m_periods = holidayPeriods;
    }

    public void removeDay(int indexToDelete) {
        m_periods.remove(indexToDelete);
    }

    /**
     * Need deep copy to support Hibernate collections
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        Holiday clone = (Holiday) super.clone();
        clone.setPeriods(new ArrayList(getPeriods()));
        return clone;
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     *
     * @param i day index
     */
    public void setDay(int i, HolidayPeriod holidayPeriod) {
        if (i >= m_periods.size()) {
            m_periods.add(holidayPeriod);
        } else {
            m_periods.set(i, holidayPeriod);
        }
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     *
     * @param i day index
     */
    public HolidayPeriod getPeriod(int i) {
        if (i < m_periods.size()) {
            return m_periods.get(i);
        }
        HolidayPeriod date = new HolidayPeriod();
        m_periods.add(date);
        return date;
    }

    /**
     * Remove all days that have indexes bigger that the one that is just passed.
     *
     * @param maxDayIndex the index of the last objects retained in the list
     */
    public void chop(int maxDayIndex) {
        if (maxDayIndex < m_periods.size()) {
            m_periods.subList(maxDayIndex + 1, m_periods.size()).clear();
        }
    }
}
