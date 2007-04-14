/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;

public class PhoneDefaultsTest extends TestCase {
    
    public void testGetProfileRotUrl() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setFullyQualifiedDomainName("pbx.sipfoundry.org");
        String actual = defaults.getProfileRootUrl();
        assertEquals("http://pbx.sipfoundry.org:8090/phone/profile/docroot", actual);
        
        String expected = "http://blah/profile";
        defaults.setProfileRootUrl(expected);
        assertEquals(expected, defaults.getProfileRootUrl());
    }
}
