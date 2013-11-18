/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.util.Map;

/**
 * Can be used to implement additional verifications for a phone log in attempt
 */
public interface LoginDelegate {
    /**
     * Runs a series of checks for a log in request
     *
     * @param user The name of the user
     * @param profile The phone profile to be used
     * @param uuid Unique id of the phone
     * @param deviceLimit The number of devices the can use the profile simultaneously (negative
     *        value means unbounded)
     * @throws IllegalArgumentException In case the log in attempt cannot be honored
     */
    void auditLoginRequest(Map<String, String> requestParams, String profile, int deviceLimit)
            throws IllegalArgumentException;
}
