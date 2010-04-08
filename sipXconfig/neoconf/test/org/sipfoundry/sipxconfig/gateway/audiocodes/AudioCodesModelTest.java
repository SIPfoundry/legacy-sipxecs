/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.device.DeviceVersion;

public class AudioCodesModelTest extends TestCase {
    public void testGetDeviceVersions() {
        DeviceVersion[] actualVersions = AudioCodesModel.getDeviceVersions();
        assertEquals("6.0", actualVersions[0].getVersionId());
        assertEquals("5.6", actualVersions[1].getVersionId());
        assertEquals("5.4", actualVersions[2].getVersionId());
        assertEquals("5.2", actualVersions[3].getVersionId());
        assertEquals("5.0", actualVersions[4].getVersionId());
    }
}
