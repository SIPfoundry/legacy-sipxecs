/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import java.util.Map;

public interface BridgeSbcXmlRpcApi {

    /**
     * Returns the registration status of each account that
     * requires registration.
     */
    public Map<String, String> getRegistrationStatus();

    /**
     * Returns the number of ongoing calls.
     *
     */
    public Integer getCallCount();
}
