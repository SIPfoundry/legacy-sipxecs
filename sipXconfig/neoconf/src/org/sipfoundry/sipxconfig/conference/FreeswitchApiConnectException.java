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

import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class FreeswitchApiConnectException extends FreeswitchApiException {
    private static final String ERROR_CONNECT = "&error.connect";

    public FreeswitchApiConnectException(Bridge bridge, Throwable cause) {
        super(ERROR_CONNECT, bridge.getHost(), cause.getMessage());
    }

    public FreeswitchApiConnectException(Location location, Throwable cause) {
        super(ERROR_CONNECT, location.getFqdn(), cause.getMessage());
    }
}
