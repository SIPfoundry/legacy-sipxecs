/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.context.ApplicationEvent;

/**
 * Signifying sipXconfig application has finished file replications
 */
public class ReplicationsFinishedEvent extends ApplicationEvent {

    public ReplicationsFinishedEvent(Object src) {
        super(src);
    }
}
