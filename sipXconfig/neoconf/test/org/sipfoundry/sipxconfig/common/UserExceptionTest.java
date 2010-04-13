/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class UserExceptionTest extends TestCase {
    public void testUserException() {
        Exception ex = new UserException();
        assertEquals("", ex.getMessage());
    }

    public void testUserExceptionMessage() {
        Exception ex = new UserException("kuku");
        assertEquals("kuku", ex.getMessage());
    }

    public void testUserExceptionMessageParam() {
        Exception ex = new UserException("ku{0}ku", new Integer(3));
        assertEquals("ku3ku", ex.getMessage());
    }

    public void testUserExceptionMessageParamParam() {
        Exception ex = new UserException("k{1}u{0}ku", new Integer(3), "bingo");
        assertEquals("kbingou3ku", ex.getMessage());
    }

    public void testUserExceptionWithCause() {
        RuntimeException exception = new RuntimeException("bongo");
        Exception ex = new UserException(exception);
        assertEquals("bongo", ex.getMessage());
    }
}
