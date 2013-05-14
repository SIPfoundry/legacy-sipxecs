/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.forwarding;

import java.util.ArrayList;
import java.util.List;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.Interval;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class Schedule extends BeanWithId {
    private User m_user;
    private String m_name;
    private String m_description;
    private WorkingTime m_workingTime;
    private Group m_userGroup;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

    public WorkingTime getWorkingTime() {
        return m_workingTime;
    }

    public void setWorkingTime(WorkingTime workingTime) {
        this.m_workingTime = workingTime;
    }

    public Group getUserGroup() {
        return m_userGroup;
    }

    public void setUserGroup(Group userGroup) {
        m_userGroup = userGroup;
    }

    public String calculateValidTime() {
        WorkingTime workingTime = getWorkingTime();
        TimeZone timeZone = (m_user != null) ? m_user.getTimezone() : TimeZone.getDefault();
        List<Interval> intervals = workingTime.calculateValidTime(timeZone);
        List<String> validTimeStr = new ArrayList<String>(intervals.size());
        for (Interval time : intervals) {
            validTimeStr.add(Integer.toHexString(time.getStart()));
            validTimeStr.add(Integer.toHexString(time.getStop()));
        }
        return StringUtils.join(validTimeStr, ':');
    }

    public void checkForValidSchedule() {
        WorkingTime workingTime = getWorkingTime();
        WorkingHours[] workingHours = workingTime.getWorkingHours();
        if (workingHours == null || workingHours.length == 0) {
            throw new ScheduleException();
        }
        workingTime.checkValid();
    }

    static class ScheduleException extends UserException {
        private static final String ERROR = "Invalid schedule. Schedule must have at least one period defined.";

        ScheduleException() {
            super(ERROR);
        }
    }
}
