/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

/**
 * RoutingExceptionTest
 */
public class RoutingExceptionTest extends TestCase {
    public void testGetCallers() {
        RoutingException exception = new RoutingException("123, 455", "922", null);
        String[] patterns = exception.getPatterns("domain.com");
        assertEquals(2, patterns.length);
        assertEquals("123@domain.com", patterns[0]);
        assertEquals("455@domain.com", patterns[1]);        
    }

    public void testGetCallersEmpty() {
        RoutingException exception = new RoutingException();
        String[] patterns = exception.getPatterns(StringUtils.EMPTY);
        assertEquals(0, patterns.length);
    }
}
