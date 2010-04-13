/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import org.springframework.context.ApplicationEvent;

/**
 * Event generated when paging server configuration is completed
 */
public class PagingServerActivatedEvent extends ApplicationEvent {

    public PagingServerActivatedEvent(Object eventSource) {
        super(eventSource);
    }
}
