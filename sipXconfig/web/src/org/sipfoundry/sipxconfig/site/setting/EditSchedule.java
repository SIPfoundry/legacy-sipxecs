/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.admin.forwarding.UserGroupSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.UserSchedule;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class EditSchedule extends UserBasePage {

    public static final String PAGE = "setting/EditSchedule";
    private static final String ACTION_ADD = "add";
    private static final String CLIENT = "client";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    @Persist(value = "session")
    public abstract String getResource();

    public abstract void setResource(String resourceId);

    @Persist
    public abstract Schedule getSchedule();

    public abstract void setSchedule(Schedule schedule);

    @Persist(value = CLIENT)
    public abstract Integer getScheduleId();

    public abstract void setScheduleId(Integer groupId);

    public abstract WorkingHours getWorkingHour();

    public abstract void setWorkingHour(WorkingHours hours);

    @Persist(value = CLIENT)
    public abstract WorkingHours[] getWorkingHours();

    public abstract void setWorkingHours(WorkingHours[] hours);

    public abstract int getIndex();

    public abstract String getAction();

    public abstract Group getUserGroup();

    public abstract void setUserGroup(Group userGroup);

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getWorkingHours() != null) {
            return;
        }

        super.pageBeginRender(event);

        WorkingHours[] workingHoursList = null;
        Schedule schedule = null;
        if (getScheduleId() != null) {
            schedule = getForwardingContext().getScheduleById(getScheduleId());
            workingHoursList = schedule.getWorkingTime().getWorkingHours();
        } else {
            if (getResource().equals("usr_sch")) {
                schedule = new UserSchedule();
                schedule.setUser(getUser());
            } else if (getResource().equals("usrGroup_sch")) {
                schedule = new UserGroupSchedule();
                schedule.setUserGroup(getUserGroup());
            } else if (getResource().equals("general_sch")) {
                schedule = new GeneralSchedule();
            }
            WorkingTime workingTime = new WorkingTime();
            workingHoursList = new WorkingHours[0];
            workingTime.setWorkingHours(workingHoursList);
            schedule.setWorkingTime(workingTime);
        }
        setSchedule(schedule);
        setWorkingHours(workingHoursList);
    }

    public void newSchedule(String resourceId, String returnPage) {
        setResource(resourceId);
        setScheduleId(null);
        setReturnPage(returnPage);
    }

    public void editSchedule(Integer scheduleId, String returnPage) {
        setResource(null);
        setScheduleId(scheduleId);
        setReturnPage(returnPage);
    }

    public void submit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        if (ACTION_ADD.equals(getAction())) {
            WorkingHours[] workingHours = getWorkingHours();
            List<WorkingHours> newWorkingHours = new ArrayList<WorkingHours>();
            for (int i = 0; i < workingHours.length; i++) {
                newWorkingHours.add(workingHours[i]);
            }
            newWorkingHours.add(new WorkingHours());
            WorkingHours[] returnedWorkingHours = new WorkingHours[0];
            setWorkingHours(newWorkingHours.toArray(returnedWorkingHours));
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        Schedule schedule = getSchedule();
        WorkingTime workingTime = schedule.getWorkingTime();
        workingTime.setWorkingHours(getWorkingHours());
        schedule.checkForValidSchedule();
        getForwardingContext().saveSchedule(schedule);
    }

    public void deletePeriod(int position) {
        WorkingHours[] workingHours = getWorkingHours();
        List<WorkingHours> newWorkingHours = new ArrayList<WorkingHours>();
        for (int i = 0; i < workingHours.length; i++) {
            newWorkingHours.add(workingHours[i]);
        }
        newWorkingHours.remove(position);
        WorkingHours[] returnedWorkingHours = new WorkingHours[0];
        setWorkingHours(newWorkingHours.toArray(returnedWorkingHours));
    }
}
