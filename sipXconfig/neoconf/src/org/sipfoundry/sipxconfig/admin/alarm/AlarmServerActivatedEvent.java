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

/**
 * Event generated when alarm server configuration is completed
 */
public class AlarmServerActivatedEvent extends ApplicationEvent {

    public AlarmServerActivatedEvent(Object eventSource) {
        super(eventSource);
    }
}
