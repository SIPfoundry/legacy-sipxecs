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

import java.util.Locale;

import junit.framework.TestCase;

import org.apache.tapestry.form.ValidationMessages;
import org.easymock.EasyMock;

public class SipxconfigNumberTranslatorTest extends TestCase {

    public void testFormatObject() {
        SipxconfigNumberTranslator out = new SipxconfigNumberTranslator();
        String zeroValue = out.format(null, Locale.US, 0);
        assertEquals("0", zeroValue);

        String oneValue = out.format(null, Locale.US, 1);
        assertEquals("1", oneValue);
    }

    public void testParse() throws Exception {
        SipxconfigNumberTranslator out = new SipxconfigNumberTranslator();
        ValidationMessages messages = EasyMock.createMock(ValidationMessages.class);
        messages.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.US).anyTimes();
        EasyMock.replay(messages);

        out.setOmitZero(true);
        Number blankValue = (Number) out.parse(null, messages, "");
        assertNull(blankValue);

        Number zeroInteger = (Number) out.parse(null, messages, "0");
        assertEquals(0, zeroInteger.intValue());

        Number oneInteger = (Number) out.parse(null, messages, "1");
        assertEquals(1, oneInteger.intValue());
    }
}
