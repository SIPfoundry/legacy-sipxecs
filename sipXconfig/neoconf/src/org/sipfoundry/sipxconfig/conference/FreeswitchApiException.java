/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.common.UserException;

public class FreeswitchApiException extends UserException {
    public FreeswitchApiException(String key, Object... args) {
        super(key, args);
    }
}
