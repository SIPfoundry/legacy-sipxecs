/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

public class DefaultProcessManagerApiProviderTest extends TestCase {

    public void testGetApi() throws Exception {
        DefaultProcessManagerApiProvider m_provider = new DefaultProcessManagerApiProvider();
        m_provider.setServiceInterface(ProcessManagerApi.class);
        m_provider.setSecure(true);
        Location location = new Location();
        location.setProcessMonitorUrl("https://host.example.org:4321");
        assertNotNull(m_provider.getApi(location));
    }
}
