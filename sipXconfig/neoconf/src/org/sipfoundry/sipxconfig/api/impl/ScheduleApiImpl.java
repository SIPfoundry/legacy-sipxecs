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
package org.sipfoundry.sipxconfig.api.impl;

import java.util.Collections;
import java.util.List;

import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.ScheduleApi;
import org.sipfoundry.sipxconfig.api.model.ScheduleBean;
import org.sipfoundry.sipxconfig.api.model.ScheduleList;
import org.sipfoundry.sipxconfig.api.model.WorkingHoursBean;
import org.sipfoundry.sipxconfig.api.model.WorkingTimeBean;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.forwarding.Schedule;
import org.sipfoundry.sipxconfig.forwarding.UserGroupSchedule;
import org.sipfoundry.sipxconfig.forwarding.UserSchedule;
import org.springframework.beans.factory.annotation.Required;

public class ScheduleApiImpl implements ScheduleApi {
    private ForwardingContext m_forwardingContext;
    private CoreContext m_coreContext;

    @Override
    public Response getAllGeneralSchedules() {
        List<GeneralSchedule> generalSchedules = m_forwardingContext.getAllGeneralSchedules();
        if (generalSchedules != null) {
            return Response.ok().entity(ScheduleList.convertScheduleList(generalSchedules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getAllUserGroupSchedules() {
        List<UserGroupSchedule> userGroupSchedules = m_forwardingContext.getAllUserGroupSchedules();
        if (userGroupSchedules != null) {
            return Response.ok().entity(ScheduleList.convertScheduleList(userGroupSchedules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getUserGroupSchedules(Integer userGroupId) {
        List<UserGroupSchedule> userGroupSchedules = m_forwardingContext.getSchedulesForUserGroupId(userGroupId);
        if (userGroupSchedules != null) {
            return Response.ok().entity(ScheduleList.convertScheduleList(userGroupSchedules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getAllUserSchedules(Integer userId) {
        User user = m_coreContext.loadUser(userId);
        List<Schedule> allUserSchedules = m_forwardingContext.getAllAvailableSchedulesForUser(user);
        if (allUserSchedules != null) {
            return Response.ok().entity(ScheduleList.convertScheduleList(allUserSchedules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPersonalUserSchedules(Integer userId) {
        List<Schedule> allUserSchedules = m_forwardingContext.getPersonalSchedulesForUserId(userId);
        if (allUserSchedules != null) {
            return Response.ok().entity(ScheduleList.convertScheduleList(allUserSchedules)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getSchedule(Integer scheduleId) {
        Schedule schedule = m_forwardingContext.getScheduleById(scheduleId);
        if (schedule != null) {
            //GroupBean groupBean = new GroupBean();
            ScheduleBean bean = ScheduleBean.convertSchedule(schedule);
            return Response.ok().entity(bean).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response newSchedule(ScheduleBean scheduleBean) {
        Schedule schedule = null;
        if (scheduleBean.getType() == ScheduleBean.ScheduleType.S) {
            schedule = new UserSchedule();
        } else if (scheduleBean.getType() == ScheduleBean.ScheduleType.U) {
            schedule = new UserGroupSchedule();
        } else {
            schedule = new GeneralSchedule();
        }
        convertToSchedule(scheduleBean, schedule);
        m_forwardingContext.saveSchedule(schedule);
        return Response.ok().entity(schedule.getId()).build();
    }

    @Override
    public Response deleteSchedule(Integer scheduleId) {
        Schedule schedule = m_forwardingContext.getScheduleById(scheduleId);
        if (schedule != null) {
            m_forwardingContext.deleteSchedulesById(Collections.singleton(scheduleId));
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response updateSchedule(Integer scheduleId, ScheduleBean scheduleBean) {
        Schedule schedule = m_forwardingContext.getScheduleById(scheduleId);
        if (schedule != null) {
            convertToSchedule(scheduleBean, schedule);
            m_forwardingContext.saveSchedule(schedule);
            return Response.ok().entity(schedule.getId()).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response addPeriod(Integer scheduleId, WorkingHoursBean whBean) {
        Schedule schedule = m_forwardingContext.getScheduleById(scheduleId);
        if (schedule != null) {
            WorkingHours wHours = WorkingHoursBean.convertToWorkingHours(whBean);
            WorkingHours[] existingWHours = schedule.getWorkingTime().getWorkingHours();
            WorkingHours[] newWorkingHours = new WorkingHours[existingWHours.length + 1];
            int i;
            for (i = 0; i < existingWHours.length; i++) {
                newWorkingHours[i] = existingWHours[i];
            }
            newWorkingHours[i] = wHours;
            schedule.getWorkingTime().setWorkingHours(newWorkingHours);
            m_forwardingContext.saveSchedule(schedule);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();

    }

    @Override
    public Response deletePeriod(Integer scheduleId, Integer index) {
        Schedule schedule = m_forwardingContext.getScheduleById(scheduleId);
        WorkingHours[] existingWHours = schedule.getWorkingTime().getWorkingHours();
        int exLength = existingWHours.length;
        WorkingHours[] wHoursToSave = new WorkingHours[exLength - 1];
        if (schedule != null && exLength > 0 && index > 0 && index < exLength) {
            for (int i = 0; i < exLength; i++) {
                int j = 0;
                if (i != index) {
                    wHoursToSave[j++] = existingWHours[i];
                }
            }
            schedule.getWorkingTime().setWorkingHours(wHoursToSave);
            m_forwardingContext.saveSchedule(schedule);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private void convertToSchedule(ScheduleBean scheduleBean, Schedule schedule) {
        schedule.setName(scheduleBean.getName());
        schedule.setDescription(scheduleBean.getDescription());
        if (schedule instanceof UserSchedule) {
            schedule.setUser(m_coreContext.getUser(scheduleBean.getUserId()));
        } else if (schedule instanceof UserGroupSchedule) {
            schedule.setUserGroup(m_coreContext.getGroupById(scheduleBean.getGroupId()));
        }
        schedule.setWorkingTime(WorkingTimeBean.convertToWorkingTime(scheduleBean.getWorkingTime()));
    }

    @Required
    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
