/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.List;

import org.sipfoundry.sipxconfig.common.User;

public interface SpeedDialManager {
    String CONTEXT_BEAN_NAME = "speedDialManager";

    SpeedDial getSpeedDialForUserId(Integer userId, boolean create);
    SpeedDial getSpeedDialForUser(User user, boolean create);

    SpeedDialGroup getSpeedDialForGroupId(Integer groupId);

    void saveSpeedDial(SpeedDial speedDial);

    void saveSpeedDialGroup(SpeedDialGroup speedDialGroup);

    void clear();

    void deleteSpeedDialsForUser(int userId);

    void deleteSpeedDialsForGroup(int groupId);

    void speedDialSynchToGroup(User user);

    boolean isSpeedDialDefinedForUserId(Integer userId);

    List<SpeedDial> findSpeedDialForUserId(Integer userId);

    SpeedDial getGroupSpeedDialForUser(User user, boolean create);
}
