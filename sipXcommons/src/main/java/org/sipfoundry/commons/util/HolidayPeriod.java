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
/**
 * if startDate is empty we default it to today
 * if endDate is empty we default it to startDate + 1 day
 */
public class HolidayPeriod {
    private Date m_startDate;
    private Date m_endDate;

    //The default starting date is today
    private Date getDefaultStartDate() {
        Calendar cal = new GregorianCalendar();
        cal.setTime(new Date());
        cal.set(Calendar.HOUR_OF_DAY, 0);
        cal.set(Calendar.MINUTE, 0);
        cal.set(Calendar.SECOND, 0);
        cal.set(Calendar.MILLISECOND, 0);
        return cal.getTime();
    }

    //By default we set a period of 1 day holiday given the starting date
    private Date getDefaultEndDate(Date startDate) {
        Calendar cal = new GregorianCalendar();
        cal.setTime(startDate);
        cal.add(Calendar.DATE, 1);
        return cal.getTime();
    }

    public Date getStartDate() {
        return m_startDate == null ? getDefaultStartDate() : m_startDate;
    }

    public void setStartDate(Date startDate) {
        m_startDate = startDate;
    }

    public Date getEndDate() {
        return m_endDate == null ? getDefaultEndDate(getStartDate()) : m_endDate;
    }

    public void setEndDate(Date endDate) {
        m_endDate = endDate;
    }
}
