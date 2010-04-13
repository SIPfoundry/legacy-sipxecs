/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendantTestUi;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class ListParkOrbitsTestUi extends ListWebTestCase {
    private File m_tempFile;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListParkOrbitsTestUi.class);
    }

    public ListParkOrbitsTestUi() throws Exception {
        super("ListParkOrbits", "resetParkOrbitContext", "orbits");
        setHasDuplicate(false);
        m_tempFile = new File(TestUtil.getTestSourceDirectory(EditAutoAttendantTestUi.class)
                + "/" + EditAutoAttendantTestUi.PROMPT_TEST_FILE);
    }

    public void setUp() {
        super.setUp();
        // sort by name
        clickLinkWithText("Name");
    }

    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:extension", "item:description",
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "orbit" + i, Integer.toString(127 + i), "orbit description + i",
        };
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        return new Object[] {
            "unchecked", paramValues[0], "Disabled", paramValues[1], m_tempFile.getName()
        };
    }

    protected void setAddParams(String[] names, String[] values) {
        super.setAddParams(names, values);
        setTextField("promptUpload", m_tempFile.getAbsolutePath());
    }

    public void testParkOrbitDefaults() {
        clickLink("orbits:defaults");
        SiteTestHelper.assertNoException(tester);
        checkCheckbox("setting:enableTimeout");
        clickButton("setting:apply");
        SiteTestHelper.assertNoException(tester);
        assertCheckboxSelected("setting:enableTimeout");
        clickButton("setting:cancel");
        assertTablePresent("orbits:list");
    }

}
