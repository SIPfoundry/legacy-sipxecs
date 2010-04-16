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

    private final boolean m_sbcsRestarted;

    public DialPlanActivatedEvent(Object eventSource, boolean sbcsRestarted) {
        super(eventSource);
        m_sbcsRestarted = sbcsRestarted;
    }

    public boolean isSbcsRestarted() {
        return m_sbcsRestarted;
    }
}
