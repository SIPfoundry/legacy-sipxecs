/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import junit.framework.TestCase;

public class GatewayTest extends TestCase {

    public void testGetCallPattern() {
        Gateway gateway = new Gateway();
        assertEquals("123", gateway.getCallPattern("123"));
        gateway.setPrefix("99");
        assertEquals("99123", gateway.getCallPattern("123"));
    }    
}
