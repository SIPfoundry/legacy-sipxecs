/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.model;

import java.util.Date;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.commons.util.HolidayPeriod;

@XmlRootElement(name = "holidayPeriods")
@XmlType(propOrder = {
        "startDate", "endDate"
        })
@JsonPropertyOrder({
        "startDate", "endDate"
    })

public class HolidayPeriodBean {
    private Date m_startDate;
    private Date m_endDate;

    public static HolidayPeriodBean convertHolidayPeriod(HolidayPeriod holidayPeriod) {
        HolidayPeriodBean holidayPeriodBean = new HolidayPeriodBean();
        holidayPeriodBean.setStartDate(holidayPeriod.getStartDate());
        holidayPeriodBean.setEndDate(holidayPeriod.getEndDate());
        return holidayPeriodBean;
    }

    public static HolidayPeriod convertToHolidayPeriod(HolidayPeriodBean holidayPeriodBean) {
        HolidayPeriod period = new HolidayPeriod();
        period.setStartDate(holidayPeriodBean.getStartDate());
        period.setEndDate(holidayPeriodBean.getEndDate());
        return period;
    }

    public Date getStartDate() {
        return m_startDate;
    }
    public void setStartDate(Date startDate) {
        m_startDate = startDate;
    }
    public Date getEndDate() {
        return m_endDate;
    }
    public void setEndDate(Date endDate) {
        m_endDate = endDate;
    }
}
