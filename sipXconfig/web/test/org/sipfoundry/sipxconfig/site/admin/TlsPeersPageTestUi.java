/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class TlsPeersPageTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(TlsPeersPageTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("toggleNavigation");
        clickLink("menu.tlspeers");
        SiteTestHelper.assertNoException(getTester());
    }

    public void testTlsPeersUi() throws Exception {
        SiteTestHelper.assertNoException(getTester());
        assertLinkPresent("link.ca");

        // check create peer page
        clickLink("peer:add");
        assertElementPresent("peer:name");
        assertCheckboxPresent("setting:900Dialing");
        assertCheckboxPresent("setting:AutoAttendant");
        assertCheckboxPresent("setting:InternationalDialing");
        assertCheckboxPresent("setting:LocalDialing");
        assertCheckboxPresent("setting:LongDistanceDialing");
        assertCheckboxPresent("setting:Mobile");
        assertCheckboxPresent("setting:TollFree");

        // have to provide a TLS peer name
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());

        // fill in the name and proceed with TLS creation
        setTextField("peer:name","peer1");
        checkCheckbox("setting:900Dialing");
        clickButton("form:ok");

        // check if back in main page
        assertLinkPresentWithExactText("peer1");

        // edit peer1
        clickLinkWithExactText("peer1");
        setTextField("peer:name","peer2");

        clickButton("form:ok");

        assertLinkPresentWithExactText("peer2");
        // create peer with same name
        clickLink("peer:add");
        setTextField("peer:name","peer2");
        clickButton("form:ok");
        SiteTestHelper.assertUserError(getTester());

        clickButton("form:cancel");
        // check if back in main page
        assertLinkPresent("link.ca");

        // delete peers
        setWorkingForm("form");
        int rowCount = SiteTestHelper.getRowCount(tester, "tlspeer:list");
        for (int i = 0; i < rowCount - 1; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("tlsPeers:delete");
        assertEquals(1, SiteTestHelper.getRowCount(tester, "tlspeer:list"));
    }

}
