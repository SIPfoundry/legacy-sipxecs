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

public class FreeswitchApiConnectException extends FreeswitchApiException {
    public FreeswitchApiConnectException(Bridge bridge, Throwable cause) {
        super("&error.connect", bridge.getHost(), cause.getMessage());
    }
}
