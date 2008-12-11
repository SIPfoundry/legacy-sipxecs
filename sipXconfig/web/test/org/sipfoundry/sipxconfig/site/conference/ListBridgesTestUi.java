/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import junit.framework.Test;

import net.sourceforge.jwebunit.html.Row;
import net.sourceforge.jwebunit.html.Table;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListBridgesTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListBridgesTestUi.class);
    }

    public ListBridgesTestUi() throws Exception {
        super("ListBridges", "resetConferenceBridgeContext", "bridge");
        setHasDuplicate(false);
    }

    @Override
    protected String[] getParamNames() {
        return new String[] {
            "item:name", "bridge:host", "item:description"
        };
    }

    @Override
    protected String[] getParamValues(int i) {
        return new String[] {
            "bridge" + i, "host" + i + ".com", "Description" + i
        };
    }

    @Override
    protected Object[] getExpectedTableRow(String[] paramValues) {
        Object[] expected = super.getExpectedTableRow(paramValues);
        expected = ArrayUtils.add(expected, 2, "Disabled");
        expected = ArrayUtils.add(expected, "0 ( )");
        return expected;
    }

    public void testAddConferenceServerWrongTab() throws Exception {
        Table expected = new Table();
        String[] values = getParamValues(0);
        addItem(getParamNames(), values);
        Row row = new Row(getExpectedTableRow(values));
        expected.appendRow(row);
        assertEquals(2, SiteTestHelper.getRowCount(tester, getTableId()));
        clickLinkWithText("bridge0");
        clickLink("link:conferences");
        SiteTestHelper.clickSubmitLink(tester, "conference:add");
        setTextField("item:extension", "44445");
        setTextField("item:name", "conference0");
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        submit("form:ok");
        SiteTestHelper.assertNoUserError(tester);
        // Add a new conference server
        clickAddLink();
        // expose BUG: only config tab present (as expected) but the displayed content is for
        // conferences tab
        assertLinkPresent("link:config");
        assertLinkNotPresent("link:conferences");
        assertLinkNotPresent("conference:add");
    }

    @Override
    public void setUp() {
        super.setUp();
        setAddLinkSubmit(true);
    }
}
