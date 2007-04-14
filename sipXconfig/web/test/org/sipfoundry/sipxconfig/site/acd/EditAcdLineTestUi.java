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
        super("acdServerPage", "resetAcdContext", "line");
        setHasDuplicate(false);
        setExactCheck(false);
    }
    
    public void setUp() {
        super.setUp();
        clickButton("form:apply");
        clickLink("link:lines");
    }

    protected String getFormId() {
        return "acdServer";
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "extension", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "line_" + i, Integer.toString(400 + i), "Description" + i
        };
    }

    public void testDisplayEdit() throws Exception {
        clickAddLink();
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
    }
}
