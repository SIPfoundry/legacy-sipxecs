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

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.User;

public interface ForwardingContext {
    static final String CONTEXT_BEAN_NAME = "forwardingContext";

    Ring getRing(Integer id);

    CallSequence getCallSequenceForUser(User user);

    // void removeCallSequenceForUserId(Integer userId);

    CallSequence getCallSequenceForUserId(Integer userId);

    void saveCallSequence(CallSequence callSequence);

    List<Schedule> getPersonalSchedulesForUserId(Integer userId);

    void saveSchedule(Schedule schedule);

    Schedule getScheduleById(Integer scheduleId);

    void deleteSchedulesById(Collection<Integer> scheduleIds);

    List<UserGroupSchedule> getSchedulesForUserGroupId(Integer userGroupId);

    List<UserGroupSchedule> getAllUserGroupSchedules();

    List<Schedule> getAllAvailableSchedulesForUser(User user);

    List<GeneralSchedule> getAllGeneralSchedules();

    List<FeatureSchedule> getAllFeatureSchedules();

    List<FeatureSchedule> getSchedulesForFeatureId(String featureId);

    void clear();

    void clearSchedules();

    /**
     * Returns true if the call sequence has rings, which would qualify it to be replicated.
     * Uses a simple plain sql select to determine that, which makes it very fast.
     * @param user
     * @return
     */
    boolean isCallSequenceReplicable(User user);
}
