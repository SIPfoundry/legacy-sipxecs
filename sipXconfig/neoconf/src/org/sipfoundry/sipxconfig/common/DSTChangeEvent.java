/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.context.ApplicationEvent;

public class DSTChangeEvent extends ApplicationEvent {

    public DSTChangeEvent(Object src) {
        super(src);
    }
}
