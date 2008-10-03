/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import org.springframework.context.ApplicationEvent;

public class AlarmsActivatedEvent extends ApplicationEvent {

    public AlarmsActivatedEvent(Object eventSource) {
        super(eventSource);
    }
}
