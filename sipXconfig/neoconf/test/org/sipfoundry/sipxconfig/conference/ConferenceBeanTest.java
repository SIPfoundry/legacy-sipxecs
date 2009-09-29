/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;

public class ConferenceBeanTest extends BeanWithSettingsTestCase {

    public void testConference() throws Exception {
        Conference conference = new Conference();
        initializeBeanWithSettings(conference);
        assertNotNull(conference.getSettings());
    }

    public void testBridge() throws Exception {
        Bridge bridge = new Bridge();
        initializeBeanWithSettings(bridge);
        assertNotNull(bridge.getSettings());
    }
}
