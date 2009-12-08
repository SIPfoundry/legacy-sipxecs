/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.common.AlarmContextImpl.ParamsUtils;

public class ParamsUtilsTest extends TestCase {
    public void testEscape() {
        String[] escaped = ParamsUtils.escape("a", "a\nb", "b\nd\t");
        assertEquals(3, escaped.length);

        assertEquals("a", escaped[0]);
        assertEquals("a&#xA;b", escaped[1]);
        assertEquals("b&#xA;d&#x9;", escaped[2]);
    }

    public void testEscapeEmpty() {
        String[] escaped = ParamsUtils.escape();
        assertEquals(0, escaped.length);
    }
}
