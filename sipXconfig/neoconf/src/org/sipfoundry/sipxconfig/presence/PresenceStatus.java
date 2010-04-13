/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;

import org.apache.commons.lang.enums.Enum;


public final class PresenceStatus extends Enum {
    public static final PresenceStatus OPEN = new PresenceStatus("open");
    public static final PresenceStatus CLOSED = new PresenceStatus("closed");
    public static final PresenceStatus NOT_AVAILABLE = new PresenceStatus("na");
    private PresenceStatus(String name) {
        super(name);
    }
    public static PresenceStatus resolve(String name) {
        return (PresenceStatus) getEnum(PresenceStatus.class, name);
    }
}
