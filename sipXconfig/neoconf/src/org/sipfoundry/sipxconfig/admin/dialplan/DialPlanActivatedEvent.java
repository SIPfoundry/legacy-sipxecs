/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.springframework.context.ApplicationEvent;

/**
 * Event generated when dial plan actication is completed
 */
public class DialPlanActivatedEvent extends ApplicationEvent {

    public DialPlanActivatedEvent(Object eventSource) {
        super(eventSource);
    }
}
