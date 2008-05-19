/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class XmlEscapeValueFilterTest extends TestCase {

    public void testFilter() {
        XmlEscapeValueFilter filter = new XmlEscapeValueFilter();

        assertNull(filter.filter(null));
    }

    public void testFilterEmpty() {
        XmlEscapeValueFilter filter = new XmlEscapeValueFilter();

        SettingValueImpl sv = new SettingValueImpl(null);
        assertSame(sv, filter.filter(sv));
    }

    public void testFilterNoXml() {
        XmlEscapeValueFilter filter = new XmlEscapeValueFilter();

        SettingValueImpl sv = new SettingValueImpl("noxml");
        assertSame(sv, filter.filter(sv));
    }

    public void testFilterXml() {
        XmlEscapeValueFilter filter = new XmlEscapeValueFilter();

        SettingValueImpl sv = new SettingValueImpl("<xml />");
        assertEquals("&lt;xml /&gt;", filter.filter(sv).getValue());

        sv = new SettingValueImpl("Bolek&Lolek");
        assertEquals("Bolek&amp;Lolek", filter.filter(sv).getValue());
    }
}
