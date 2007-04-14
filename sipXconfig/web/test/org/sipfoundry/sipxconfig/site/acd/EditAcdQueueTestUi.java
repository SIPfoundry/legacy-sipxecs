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

public class EditAcdQueueTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAcdQueueTestUi.class);
    }

    public EditAcdQueueTestUi() {
        super("acdServerPage", "resetAcdContext", "queue");
        setHasDuplicate(false);
        setExactCheck(false);
    }
    
    public void setUp() {
        super.setUp();
        clickButton("form:apply");
        clickLink("link:queues");        
    }

    protected String getFormId() {
        return "acdServer";
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "queue_" + i, "Description" + i
        };
    }

    protected void setAddParams(String[] names, String[] values) {
        super.setAddParams(names, values);
        SiteTestHelper.initUploadFields(getDialog().getForm(), "EditAcdQueueTestUi");
    }

    public void testDisplayEdit() throws Exception {
        clickAddLink();
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
        assertLinkNotPresent("link:config");
        assertLinkNotPresent("link:agents");
        
        setAddParams(getParamNames(), getParamValues(101));
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
        assertLinkPresent("link:config");
        assertLinkPresent("link:agents");        
    }
}
