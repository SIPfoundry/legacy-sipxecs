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

import org.codehaus.jackson.annotate.JsonProperty;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;

@XmlRootElement(name = "workingHours")
public class WorkingTimeBean {
    private List<WorkingHoursBean> m_workingHours;

    public void setWorkingHours(List<WorkingHoursBean> workingHours) {
        m_workingHours = workingHours;
    }

    public static WorkingTimeBean convertWorkingTimeBean(WorkingTime workingTime) {
        List<WorkingHoursBean> workingHoursList = new ArrayList<WorkingHoursBean>();
        for (WorkingHours wHours : workingTime.getWorkingHours()) {
            workingHoursList.add(WorkingHoursBean.convertWorkingHours(wHours));
        }
        WorkingTimeBean workingTimeBean = new WorkingTimeBean();
        workingTimeBean.setWorkingHours(workingHoursList);
        return workingTimeBean;
    }

    public static WorkingTime convertToWorkingTime(WorkingTimeBean workingTimeBean) {
        WorkingTime wTime = new WorkingTime();
        WorkingHours[] workingHoursArray = new WorkingHours[workingTimeBean.getWorkingTime().size()];
        int i = 0;
        for (WorkingHoursBean wHoursBean : workingTimeBean.getWorkingTime()) {
            workingHoursArray[i++] = (WorkingHoursBean.convertToWorkingHours(wHoursBean));
        }
        wTime.setWorkingHours(workingHoursArray);
        return wTime;
    }

    @XmlElement(name = "workingHours")
    @JsonProperty(value = "workingHours")
    public List<WorkingHoursBean> getWorkingTime() {
        if (m_workingHours == null) {
            m_workingHours = new ArrayList<WorkingHoursBean>();
        }
        return m_workingHours;
    }
}
