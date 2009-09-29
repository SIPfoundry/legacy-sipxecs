/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.AssertionFailedError;
import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

public class InternalErrorPageTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(InternalErrorPageTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
    }

    public void testShowExceptionPage() throws Exception {
        clickLink("ShowExceptionPage");
        try {
            SiteTestHelper.assertNoException(tester);
            // cannot call fail here it'll get caught by next catch
            throw new IllegalArgumentException();
        } catch (AssertionFailedError e) {
            // passed
        } catch (IllegalArgumentException e) {
            fail("SiteTestHelper.assertNoException did not properly detect the error page");
        }
    }
}
