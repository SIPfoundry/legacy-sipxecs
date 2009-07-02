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

public interface ForwardingContext extends AliasProvider {
    static final String CONTEXT_BEAN_NAME = "forwardingContext";

    Ring getRing(Integer id);

    CallSequence getCallSequenceForUser(User user);

    void removeCallSequenceForUserId(Integer userId);

    CallSequence getCallSequenceForUserId(Integer userId);

    void saveCallSequence(CallSequence callSequence);

    void flush();

    List<Schedule> getPersonalSchedulesForUserId(Integer userId);

    void saveSchedule(Schedule schedule);

    Schedule getScheduleById(Integer scheduleId);

    void deleteSchedulesById(Collection<Integer> scheduleIds);

    void notifyCommserver();

    List<UserGroupSchedule> getSchedulesForUserGroupId(Integer userGroupId);

    List<UserGroupSchedule> getAllUserGroupSchedules();

    List<Schedule> getAllAvailableSchedulesForUser(User user);

    List<GeneralSchedule> getAllGeneralSchedules();

    void clear();

    void clearSchedules();
}
