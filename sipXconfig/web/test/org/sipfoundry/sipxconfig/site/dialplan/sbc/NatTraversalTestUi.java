/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.dialplan.sbc;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class NatTraversalTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NatTraversalTestUi.class);
    }

    @Override
    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("initNatTraversal");
        clickLink("InternetCalling");
        SiteTestHelper.assertNoUserError(tester);
        clickLink("link:natTraversal");
    }

    public void testApplySettings() {
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("natTraversal:apply");
        SiteTestHelper.clickSubmitLink(tester, "setting:toggle");
        checkCheckbox("enabled");
        clickButton("natTraversal:apply");
        SiteTestHelper.assertNoUserError(tester);
        tester.assertCheckboxSelected("enabled");
    }
}
