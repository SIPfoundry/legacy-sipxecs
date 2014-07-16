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

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.forwarding.Schedule;

@XmlRootElement(name = "Schedules")
public class ScheduleList {

    private List<ScheduleBean> m_schedules;

    public void setSchedules(List<ScheduleBean> schedules) {
        m_schedules = schedules;
    }

    public static ScheduleList convertScheduleList(List<? extends Schedule> schedules) {
        List<ScheduleBean> scheduleList = new ArrayList<ScheduleBean>();
        for (Schedule schedule : schedules) {
            scheduleList.add(ScheduleBean.convertSchedule(schedule));
        }
        ScheduleList list = new ScheduleList();
        list.setSchedules(scheduleList);
        return list;
    }

    @XmlElement(name = "Schedule")
    public List<ScheduleBean> getSchedules() {
        if (m_schedules == null) {
            m_schedules = new ArrayList<ScheduleBean>();
        }
        return m_schedules;
    }
}
