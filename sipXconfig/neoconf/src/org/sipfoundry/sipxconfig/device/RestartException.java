/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import org.sipfoundry.sipxconfig.common.UserException;

/**
 * System was unable to restart phone
 */
public class RestartException extends UserException {
    public RestartException(String msg) {
        super(msg);
    }
}
