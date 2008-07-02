/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.service;

import junit.framework.TestCase;

public class SipxServiceTestBase extends TestCase {
    protected void initCommonAttributes(SipxService service) {
        service.setIpAddress("192.168.1.1");
        service.setHostname("sipx");
        service.setFullHostname("sipx.example.org");
        service.setDomainName("example.org");
        service.setRealm("realm.example.org");
        service.setSipPort("5060");
    }
}
