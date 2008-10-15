/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListAcdServersTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListAcdServersTestUi.class);
    }

    public ListAcdServersTestUi() {
        super("listAcdServers", "resetAcdContext", "server");
        setHasDuplicate(false);
        setExactCheck(true);
        setAddLinkSubmit(true);
    }

    protected String[] getParamNames() {
        return new String[] {
            "hostField", "portField"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "localhost" + i, Integer.toString(8100 + i)
        };
    }

    public void testDisplayEdit() throws Exception {
        clickAddLink();
        SiteTestHelper.assertNoException(tester);
        setWorkingForm("form");
        assertFormElementPresent("hostField");
        assertFormElementPresent("portField");
        assertFormElementPresent("form:ok");
        assertFormElementPresent("form:cancel");
    }

    public void testPresenceServerLink() throws Exception {
        clickLink("link:presence");
        assertElementPresent("setting:SIP_PRESENCE_SIGN_IN_CODE");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
    }
}
