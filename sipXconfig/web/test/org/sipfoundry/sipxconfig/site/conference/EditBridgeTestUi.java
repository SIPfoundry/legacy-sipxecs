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

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendantTestUi;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class EditBridgeTestUi extends ListWebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditBridgeTestUi.class);
    }

    public EditBridgeTestUi() {
        super("EditBridge", "resetConferenceBridgeContext", "conference");
        setHasDuplicate(false);
        setExactCheck(false);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("resetConferenceBridgeContext");
        clickLink("EditBridge");
        setWorkingForm("form");
        setFormElement("name", "bridge_test");
        SiteTestHelper.initUploadFieldsWithFile(getDialog().getForm(), TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        clickButton("form:apply");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "extension", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "conference" + i, "444" + i, "Description" + i
        };
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        return ArrayUtils.add(paramValues, 1, "false");
    }

    protected void clickAddLink() throws Exception {
        SiteTestHelper.initUploadFieldsWithFile(getDialog().getForm(), TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        super.clickAddLink();
    }

    protected void clickDeleteButton() {
        SiteTestHelper.initUploadFieldsWithFile(getDialog().getForm(), TestUtil
                .getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
        super.clickDeleteButton();
    }
}
