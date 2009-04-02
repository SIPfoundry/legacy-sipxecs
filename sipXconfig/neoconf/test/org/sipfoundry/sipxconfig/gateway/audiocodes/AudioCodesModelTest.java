/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.device.DeviceVersion;

public class AudioCodesModelTest extends TestCase {

    public void testCleanSerialNumber() {
        // no tests here - at least for now
    }

    public void testGetDeviceVersions() {
        AudioCodesModel audioCodesModel = new AudioCodesModel();
       
        DeviceVersion[] actualVersions = audioCodesModel.getDeviceVersions();
        assertEquals("5.4", actualVersions[0].getVersionId());
        assertEquals("5.2", actualVersions[1].getVersionId());
        assertEquals("5.0", actualVersions[2].getVersionId());
    }
}
