/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class Holiday extends ScheduledAttendant {
    private List<Date> m_dates = new ArrayList<Date>();

    public void addDay(Date day) {
        m_dates.add(day);
    }

    public void removeDay(Date day) {
        m_dates.remove(day);
    }

    public List<Date> getDates() {
        return m_dates;
    }

    public void setDates(List dates) {
        m_dates = dates;
    }

    public void removeDay(int indexToDelete) {
        m_dates.remove(indexToDelete);
    }

    /**
     * Need deep copy to support Hibernate collections
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        Holiday clone = (Holiday) super.clone();
        clone.setDates(new ArrayList(getDates()));
        return clone;
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     *
     * @param i day index
     */
    public void setDay(int i, Date holidayDay) {
        if (i >= m_dates.size()) {
            m_dates.add(holidayDay);
        } else {
            m_dates.set(i, holidayDay);
        }
    }

    /**
     * It's safe to call this function even if the index is bigger than current number of days
     *
     * @param i day index
     */
    public Date getDay(int i) {
        if (i < m_dates.size()) {
            return m_dates.get(i);
        }
        Date date = new Date();
        m_dates.add(date);
        return date;
    }

    /**
     * Remove all days that have indexes bigger that the one that is just passed.
     *
     * @param maxDayIndex the index of the last objects retained in the list
     */
    public void chop(int maxDayIndex) {
        if (maxDayIndex < m_dates.size()) {
            m_dates.subList(maxDayIndex + 1, m_dates.size()).clear();
        }
    }
}
