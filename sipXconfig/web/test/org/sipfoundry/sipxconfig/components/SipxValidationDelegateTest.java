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

import java.util.Locale;

import junit.framework.TestCase;

import org.apache.hivemind.impl.AbstractMessages;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.UserException;

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

    public void testRecord() {
        UserException exception = new UserException("Error message");

        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.record(exception, null);
        assertEquals("Error message", delegate.getFirstError().toString());
    }

    public void testRecordWithMessage() {
        UserException exception = new UserException("&error");

        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.record(exception, new DummyMessages());
        assertEquals("Error message", delegate.getFirstError().toString());
    }

    public void testRecordWithMessageAndParams() {
        UserException exception = new UserException("&machine", "&raven", "10.1.3.4", 2);

        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.record(exception, new DummyMessages());
        assertEquals("Nazwa: kruk, IP: 10.1.3.4, Numer: 2", delegate.getFirstError().toString());
    }

    public void testRecordWithMissingMessage() {
        UserException exception = new UserException("Another error");

        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.record(exception, new DummyMessages());
        assertEquals("Another error", delegate.getFirstError().toString());
    }

    public void testRecordWithMessageAndCause() {
        RuntimeException cause = new RuntimeException("Something happened");
        UserException exception = new UserException(cause);

        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.record(exception, new DummyMessages());
        assertEquals("Something happened", delegate.getFirstError().toString());
    }

    private class DummyMessages extends AbstractMessages {
        @Override
        protected Locale getLocale() {
            return Locale.US;
        }

        @Override
        protected String findMessage(String key) {
            if (key.equals("error")) {
                return "Error message";
            }
            if (key.equals("machine")) {
                return "Nazwa: {0}, IP: {1}, Numer: {2}";
            }
            if (key.equals("raven")) {
                return "kruk";
            }
            return null;
        }
    }
}
