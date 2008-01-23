/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.apache.commons.collections.Predicate;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class EmergencyPhoneModelSourceTest extends TestCase {
    
    public void testPredicate() {
        Predicate actual = EmergencyPhoneModelSource.getPredicate();
        assertTrue(actual.evaluate(new PolycomModel()));
        assertFalse(actual.evaluate(new PhoneModel()));
    }
}
