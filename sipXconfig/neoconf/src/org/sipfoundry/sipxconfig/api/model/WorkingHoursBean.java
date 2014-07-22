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

import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.apache.commons.lang.enums.EnumUtils;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.common.ScheduledDay;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;

@XmlRootElement(name = "workingHours")
@XmlType(propOrder = {
        "enabled", "start", "stop", "scheduledDay"
        })
@JsonPropertyOrder({
        "enabled", "start", "stop", "scheduledDay"
    })

public class WorkingHoursBean {
    private static final String SPACE = " ";
    private static final String UNDERSCORE = "_";
    private boolean m_enabled;
    private Date m_start;
    private Date m_stop;
    private ScheduledDayBean m_scheduledDay;

    @XmlType(name = "scheduledDay")
    @XmlEnum
    public enum ScheduledDayBean {
        @XmlEnumValue(value = "WEEKEND")
        Weekend,
        @XmlEnumValue(value = "WEEKDAYS")
        Weekdays,
        @XmlEnumValue(value = "EVERYDAY")
        Every_day,
        @XmlEnumValue(value = "SUNDAY")
        Sunday,
        @XmlEnumValue(value = "MONDAY")
        Monday,
        @XmlEnumValue(value = "TUESDAY")
        Tuesday,
        @XmlEnumValue(value = "WEDNESDAY")
        Wednesday,
        @XmlEnumValue(value = "THURSDAY")
        Thursday,
        @XmlEnumValue(value = "FRIDAY")
        Friday,
        @XmlEnumValue(value = "SATURDAY")
        Saturday
    }

    public static WorkingHoursBean convertWorkingHours(WorkingHours workingHours) {
        WorkingHoursBean workingHoursBean = new WorkingHoursBean();
        workingHoursBean.setEnabled(workingHours.isEnabled());
        workingHoursBean.setStart(workingHours.getStart());
        workingHoursBean.setStop(workingHours.getStop());
        workingHoursBean.setScheduledDay(ScheduledDayBean.
            valueOf(workingHours.getDay().getName().replace(SPACE, UNDERSCORE)));
        return workingHoursBean;
    }

    public static WorkingHours convertToWorkingHours(WorkingHoursBean workingHoursBean) {
        WorkingHours workingHours = new WorkingHours();
        workingHours.setEnabled(workingHoursBean.isEnabled());
        workingHours.setStart(workingHoursBean.getStart());
        workingHours.setStop(workingHoursBean.getStop());
        workingHours.setDay((ScheduledDay) EnumUtils.
            getEnum(ScheduledDay.class, workingHoursBean.getScheduledDay().name().replace(UNDERSCORE, SPACE)));
        return workingHours;
    }

    public boolean isEnabled() {
        return m_enabled;
    }
    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
    public ScheduledDayBean getScheduledDay() {
        return m_scheduledDay;
    }
    public void setScheduledDay(ScheduledDayBean scheduledDay) {
        m_scheduledDay = scheduledDay;
    }
    public Date getStart() {
        return m_start;
    }
    public void setStart(Date start) {
        m_start = start;
    }
    public Date getStop() {
        return m_stop;
    }
    public void setStop(Date stop) {
        m_stop = stop;
    }
}
