/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.text.DecimalFormat;
import java.util.Locale;

import junit.framework.TestCase;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.ValidationMessagesImpl;
import org.apache.tapestry.valid.ValidatorException;

public class IntTranslatorTest extends TestCase {

    public void testGetDecimalFormat() {
        IntTranslator out = new IntTranslator();
        DecimalFormat format = out.getDecimalFormat(Locale.US);
        assertNotNull(format);
        assertFalse(format.isGroupingUsed());
    }

    public void testFormat() {
        int testInt = 3456;
        IntTranslator out = new IntTranslator();
        String formattedInt = out.format(null, Locale.US, testInt);
        assertEquals("3456", formattedInt);
    }

    public void testParseText() {
        IntTranslator out = new IntTranslator();
        IFormComponent field = new TextFieldMock();
        ValidationMessages messages = new ValidationMessagesImpl(field, Locale.US);

        long result = 0;
        try {
            String text = "123";
            result = (Long) out.parseText(field, messages, text);
        } catch (ValidatorException ex) {
        }
        assertEquals(123, result);

        result = 0;
        try {
            String text = "123abc";
            out.parseText(field, messages, text);
        } catch (ValidatorException ex) {
            result = -1;
        }
        assertEquals(-1, result);

    }
}
