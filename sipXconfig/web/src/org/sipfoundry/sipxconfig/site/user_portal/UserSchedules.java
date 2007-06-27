/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditSchedule;
import org.sipfoundry.sipxconfig.site.user.ManageUsers;

public abstract class UserSchedules extends UserBasePage implements PageBeginRenderListener {

    public static final String PAGE = "UserSchedules";

    public static final String USER_SCH_RESOURCE_ID = "usr_sch";

    @InjectObject(value = "spring:forwardingContext")
    public abstract ForwardingContext getForwardingContext();

    public abstract void setSchedules(List groups);

    public abstract List getSchedules();

    public abstract SettingDao getSettingContext();

    public abstract void setSchedule(Schedule sch);

    public abstract Schedule getSchedule();

    public abstract SelectMap getSelections();

    public abstract boolean getChanged();

    public IPage addSchedule(IRequestCycle cycle) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.setUserId(getUserId());
        page.newSchedule(USER_SCH_RESOURCE_ID, PAGE);
        return page;
    }

    public IPage editSchedulesGroup(IRequestCycle cycle, Integer scheduleId) {
        EditSchedule page = (EditSchedule) cycle.getPage(EditSchedule.PAGE);
        page.editSchedule(scheduleId, PAGE);
        return page;
    }

    public IPage showGroupMembers(IRequestCycle cycle, Integer groupId) {
        ManageUsers page = (ManageUsers) cycle.getPage(ManageUsers.PAGE);
        page.setGroupId(groupId);
        return page;
    }

    public void pageBeginRender(PageEvent event) {
        if (getChanged()) {
            setSchedules(null);
        }

        if (getSchedules() != null) {
            return;
        }

        super.pageBeginRender(event);

        Integer userId = getUserId();
        ForwardingContext forwardingContext = getForwardingContext();
        List<Schedule> schedules = forwardingContext.getSchedulesForUserId(userId);
        setSchedules(schedules);
    }
}
