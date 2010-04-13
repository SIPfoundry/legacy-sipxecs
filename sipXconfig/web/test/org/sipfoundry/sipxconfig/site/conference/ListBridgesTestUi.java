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

import static org.sipfoundry.sipxconfig.site.SiteTestHelper.getBaseUrl;
import junit.framework.Test;
import net.sourceforge.jwebunit.html.Row;
import net.sourceforge.jwebunit.html.Table;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

public class ListBridgesTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListBridgesTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("resetConferenceBridgeContext");
    }

    public void testDisplay() {
        clickLink("ListBridges");
        SiteTestHelper.assertNoException(tester);
        assertFormPresent();
        assertEquals(1, SiteTestHelper.getRowCount(tester, "bridge:list"));
        assertElementPresentByXPath("//input[@type = 'submit' and @id='refresh']");
    }

    public void testAdd() {
        ConferenceTestHelper helper = new ConferenceTestHelper(tester);
        helper.createBridge();
        SiteTestHelper.home(tester);
        clickLink("ListBridges");
        SiteTestHelper.assertNoException(tester);
        assertFormPresent();
        assertEquals(2, SiteTestHelper.getRowCount(tester, "bridge:list"));

        Table expected = new Table();
        Row expectedRow =
            new Row(new Object[]{ "unchecked", TestPage.TEST_LOCATION_FQDN, TestPage.TEST_LOCATION_NAME, "0 ( )" });
        expected.appendRow(expectedRow);
        assertTableRowsEqual("bridge:list", 1, expected);

    }
}
