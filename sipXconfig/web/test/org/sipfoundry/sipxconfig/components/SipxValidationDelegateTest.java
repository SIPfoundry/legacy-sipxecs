/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.tapestry.valid.ValidationConstraint;

public class SipxValidationDelegateTest extends TestCase {
    public void testGetHasSuccess() {
        SipxValidationDelegate delegate = new SipxValidationDelegate();
        assertFalse(delegate.getHasSuccess());
        delegate.recordSuccess("bongo");
        assertTrue(delegate.getHasSuccess());
        assertEquals("bongo", delegate.getSuccess());
        delegate.clear();
        assertFalse(delegate.getHasSuccess());
    }

    public void testGetHasSuccessWithErrors() {
        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.recordSuccess("bongo");
        delegate.record("error", ValidationConstraint.CONSISTENCY);
        assertFalse(delegate.getHasSuccess());
    }    
}
