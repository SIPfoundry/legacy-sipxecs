/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.common.User;

/**
 * ForwardingContext
 */
public interface ForwardingContext extends AliasProvider {
    public static final String CONTEXT_BEAN_NAME = "forwardingContext";

    public Ring getRing(Integer id);

    public CallSequence getCallSequenceForUser(User user);

    public void removeCallSequenceForUserId(Integer userId);

    public CallSequence getCallSequenceForUserId(Integer userId);

    public void saveCallSequence(CallSequence callSequence);

    public void clear();

    public void flush();

    public List<Schedule> getPersonalSchedulesForUserId(Integer userId);

    public void saveSchedule(Schedule schedule);

    public Schedule getScheduleById(Integer scheduleId);

    public List<Schedule> deleteSchedulesById(Collection<Integer> scheduleIds);

    public void notifyCommserver();

    public List<UserGroupSchedule> getSchedulesForUserGroupId(Integer userGroupId);

    public List<UserGroupSchedule> getAllUserGroupSchedules();

    public List<Schedule> getAllAvailableSchedulesForUser(User user);
}
