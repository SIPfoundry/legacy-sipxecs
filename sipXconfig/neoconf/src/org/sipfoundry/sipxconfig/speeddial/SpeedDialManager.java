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

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;

public interface SpeedDialManager extends DialingRuleProvider {
    String CONTEXT_BEAN_NAME = "speedDialManager";

    SpeedDial getSpeedDialForUserId(Integer userId, boolean create);

    SpeedDialGroup getSpeedDialForGroupId(Integer groupId);

    void saveSpeedDial(SpeedDial speedDial);

    void saveSpeedDialGroup(SpeedDialGroup speedDialGroup);

    void activateResourceList();

    void clear();

    void deleteSpeedDialsForUser(int userId);

    boolean isSpeedDialDefinedForUserId(Integer userId);
}
