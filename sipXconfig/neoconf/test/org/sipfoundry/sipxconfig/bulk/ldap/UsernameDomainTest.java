/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.bulk.ldap;

import static org.sipfoundry.commons.security.Util.retrieveDomain;
import static org.sipfoundry.commons.security.Util.retrieveUsername;
import junit.framework.TestCase;

public class UsernameDomainTest extends TestCase {

    public void testRetrieveUsernameDomain() {
        String username = retrieveUsername("joe@example.com");
        String domain = retrieveDomain("joe@example.com");
        assertEquals("joe", username);
        assertEquals("example.com", domain);

        username = retrieveUsername("example.com\\joe");
        domain = retrieveDomain("example.com\\joe");
        assertEquals("joe", username);
        assertEquals("example.com", domain);

        username = retrieveUsername("joe");
        domain = retrieveDomain("joe");
        assertEquals("joe", username);
        assertNull(domain);
    }
}
