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
}
