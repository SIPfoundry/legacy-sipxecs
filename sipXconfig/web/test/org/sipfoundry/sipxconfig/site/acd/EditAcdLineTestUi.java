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

public class EditAcdLineTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAcdLineTestUi.class);
    }

    public EditAcdLineTestUi() {
        super("listAcdServers", "resetAcdContext", "line");
        setHasDuplicate(false);
        setExactCheck(false);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, false);
        clickLink("seedAcdServer");
        clickLink("listAcdServers");
        clickLink("editRowLink");
        clickLink("link:lines");
    }

    protected String getFormId() {
        return "acdServer";
    }

    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:extension", "item:description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "line_" + i, Integer.toString(400 + i), "Description" + i
        };
    }
}
