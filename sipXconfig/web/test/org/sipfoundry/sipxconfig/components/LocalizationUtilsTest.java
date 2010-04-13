/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Locale;

import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;
import org.sipfoundry.sipxconfig.common.UserException;

import junit.framework.TestCase;

public class LocalizationUtilsTest extends TestCase {

    public void testGetMessages() throws Exception {
        Messages messages = new DummyMessages();

        assertEquals("cde", LocalizationUtils.getMessage(messages, "abc", "cde"));
        assertNull("cde", LocalizationUtils.getMessage(messages, "xyz", null));
        assertEquals("dummy label", LocalizationUtils.getMessage(messages, "dummy", null));
    }

    public void testLocalizeString() {
        Messages messages = new DummyMessages();

        assertNull(LocalizationUtils.localizeString(messages, null));
        assertEquals("dummy", LocalizationUtils.localizeString(messages, "dummy"));
        assertEquals("bongo", LocalizationUtils.localizeString(messages, "bongo"));
        assertEquals("dummy label", LocalizationUtils.localizeString(messages, "&dummy"));
        assertEquals("[BONGO]", LocalizationUtils.localizeString(messages, "&bongo"));
    }

    public void testLocalizeArray() {
        Messages messages = new DummyMessages();

        Object[] array = null;
        assertNull(LocalizationUtils.localizeArray(messages, array));
        assertEquals(0, LocalizationUtils.localizeArray(messages).length);

        Object[] result = LocalizationUtils.localizeArray(messages, "dummy", 4, "bongo", "&dummy", "&bongo", null);

        assertEquals(6, result.length);
        assertEquals("dummy", result[0]);
        assertEquals(4, result[1]);
        assertEquals("bongo", result[2]);
        assertEquals("dummy label", result[3]);
        assertEquals("[BONGO]", result[4]);
        assertNull(result[5]);
    }

    public void testLocalizeException() {
        Messages messages = new DummyMessages();
        UserException e1 = new UserException("&dummy");
        assertEquals("dummy label", LocalizationUtils.localizeException(messages, e1));

        UserException e2 = new UserException("dummy");
        assertEquals("dummy", LocalizationUtils.localizeException(messages, e2));

        UserException e3 = new UserException("&fmt", "&param", 7);
        assertEquals("This is <param label> - 7", LocalizationUtils.localizeException(messages, e3));
    }

    private class DummyMessages extends AbstractMessages {
        @Override
        protected Locale getLocale() {
            return Locale.US;
        }

        @Override
        protected String findMessage(String key) {
            if (key.equals("dummy")) {
                return "dummy label";
            }
            if (key.equals("param")) {
                return "param label";
            }
            if (key.equals("fmt")) {
                return "This is <{0}> - {1}";
            }
            return null;
        }
    }
}
