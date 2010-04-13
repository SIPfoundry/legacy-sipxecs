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
import net.sourceforge.jwebunit.html.Row;
import net.sourceforge.jwebunit.html.Table;

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditAcdQueueTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditAcdQueueTestUi.class);
    }

    public EditAcdQueueTestUi() {
        super("listAcdServers", "resetAcdContext", "queue");
        setHasDuplicate(false);
        setExactCheck(false);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.setScriptingEnabled(tester, true);
        clickLink("seedAcdServer");
        clickLink("listAcdServers");
        clickLink("editRowLink");
        clickLink("link:queues");
    }

    protected String getFormId() {
        return "acdServer";
    }

    protected String[] getParamNames() {
        return new String[] {
            "item:name", "item:description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "queue_" + i, "Description" + i
        };
    }

    protected void setAddParams(String[] names, String[] values) {
        super.setAddParams(names, values);
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

    public void testDisplayEditWithOverflowQueue() throws Exception {
        clickAddLink();

        setAddParams(new String[] {
            "item:name", "item:description"
        }, new String[] {
            "Q1", "description 1"
        });
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        clickAddLink();
        setAddParams(new String[] {
            "item:name", "item:description"
        }, new String[] {
            "Q2", "description 2"
        });
        selectOption("setting:overflow-type", "Queue");

        // re-initialize upload fields as previous set refreshes page

        selectOption("setting:overflow-typeValue", "Q1");
        clickButton("form:ok");
        SiteTestHelper.assertNoUserError(tester);

        assertEquals(3, SiteTestHelper.getRowCount(tester, getTableId()));
        setWorkingForm("queuesForm");
        SiteTestHelper.clickSubmitLink(tester, "linkColumn");
        Table expectedTable = new Table();
        expectedTable.appendRow(new Row(new String[] {
            "unchecked", "Q1", "description 1", ""
        }));
        expectedTable.appendRow(new Row(new String[] {
            "unchecked", "Q2", "description 2", "Q1"
        }));

        assertTableRowsEqual(getTableId(), 1, expectedTable);
    }
}
