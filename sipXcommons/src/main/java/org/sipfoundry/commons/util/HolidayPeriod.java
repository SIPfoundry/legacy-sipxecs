/**
 *
 *
 * Copyright (c) 2013 Karel, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.commons.util;

import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;

public class HolidayPeriod {

    private Date m_startDate;
    private Date m_endDate;

    public HolidayPeriod() {
        super();
        m_startDate = getToday();
        m_endDate = getTomorow();
    }

    private Calendar getStartOfDayCalendar() {
        Calendar cal = new GregorianCalendar();
        cal.set(Calendar.HOUR_OF_DAY, 0);
        cal.set(Calendar.MINUTE, 0);
        cal.set(Calendar.SECOND, 0);
        cal.set(Calendar.MILLISECOND, 0);
        return cal;
    }

    private Date getToday() {
        Calendar cal = getStartOfDayCalendar();
        return cal.getTime();
    }

    private Date getTomorow() {
        Calendar cal = getStartOfDayCalendar();
        cal.add(Calendar.DATE, 1);
        return cal.getTime();
    }

    public Date getStartDate() {
        return m_startDate;
    }

    public void setStartDate(Date startDate) {
        this.m_startDate = startDate;
    }

    public Date getEndDate() {
        return m_endDate;
    }

    public void setEndDate(Date endDate) {
        this.m_endDate = endDate;
    }

}
