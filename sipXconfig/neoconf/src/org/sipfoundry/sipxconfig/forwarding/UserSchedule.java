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


public class UserSchedule extends Schedule {

    @Override
    public String getEntityIdentifier() {
        return getUser().getEntityIdentifier();
    }

    @Override
    public String getConfigChangeType() {
        return getClass().getSimpleName();
    }

}
