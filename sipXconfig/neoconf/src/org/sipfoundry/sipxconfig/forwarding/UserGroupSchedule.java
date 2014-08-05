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

import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;

public class UserGroupSchedule extends Schedule {

    @Override
    public String getEntityIdentifier() {
        return getUserGroup().getEntityIdentifier();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.GROUP_SCHEDULE;
    }

}
