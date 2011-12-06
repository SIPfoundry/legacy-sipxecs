/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

public class RegistrationMetricsTest extends TestCase {
    private RegistrationMetrics m_metrics;

    protected void setUp() {
        m_metrics = new RegistrationMetrics();
    }

    public void testUniqueRegistrations() {
        String[][] regData = {
                {
                    "contact1", "10"
                }, {
                    "contact2", "11"
                }, {
                    "contact1", "12"
                }, {
                    "contact3", "13"
                }, {
                    "contact2", "9"
                }, {
                    "contact2", "11"
                }
            };
            List regs = new ArrayList();
            for (int i = 0; i < regData.length; i++) {
                RegistrationItem item = new RegistrationItem();
                item.setContact(regData[i][0]);
                item.setExpires(Long.parseLong(regData[i][1]));
                regs.add(item);
            }

            m_metrics.setRegistrations(regs);
            List cleanRegs = new ArrayList(m_metrics.getUniqueRegistrations());
            assertEquals(3, cleanRegs.size());
            assertEquals("contact1", ((RegistrationItem) cleanRegs.get(0)).getContact());
            assertEquals("contact2", ((RegistrationItem) cleanRegs.get(1)).getContact());
            assertEquals("contact3", ((RegistrationItem) cleanRegs.get(2)).getContact());
            assertEquals(12, ((RegistrationItem) cleanRegs.get(0)).getExpires());
            assertEquals(11, ((RegistrationItem) cleanRegs.get(1)).getExpires());
            assertEquals(13, ((RegistrationItem) cleanRegs.get(2)).getExpires());
    }

    public void testCalculateMetricsEmpty() {
        RegistrationMetrics metrics = new RegistrationMetrics();
        assertTrue(1.0 == metrics.getLoadBalance());

        metrics.setRegistrations(Collections.EMPTY_LIST);
        assertTrue(1.0 == metrics.getLoadBalance());
    }

    public void testCalculateMetricsSingleMachine() {
        RegistrationItem[] items = new RegistrationItem[] {
                newRegistrationItem("mallard"),
                newRegistrationItem("mallard"),
                newRegistrationItem("mallard"),
                newRegistrationItem("mallard")
        };
        RegistrationMetrics metrics = new RegistrationMetrics();
        metrics.setRegistrations(Arrays.asList(items));
        assertTrue(1.0 == metrics.getLoadBalance());
    }

    public void testCalculateExcellentMetrics() {
        RegistrationItem[] items = new RegistrationItem[] {
                newRegistrationItem("mallard"),
                newRegistrationItem("mallard"),
                newRegistrationItem("bigbird"),
                newRegistrationItem("bigbird")
        };
        RegistrationMetrics metrics = new RegistrationMetrics();
        metrics.setUniqueRegistrations(Arrays.asList(items));
        assertTrue(2.0 == metrics.getLoadBalance());
    }

    public void testCalculateGoodMetrics() {
        RegistrationItem[] items = new RegistrationItem[] {
                newRegistrationItem("mallard"),
                newRegistrationItem("bigbird"),
                newRegistrationItem("bigbird"),
                newRegistrationItem("bigbird"),
                newRegistrationItem("bigbird")
        };
        RegistrationMetrics metrics = new RegistrationMetrics();
        metrics.setUniqueRegistrations(Arrays.asList(items));
        assertTrue(1.4705882352941173 == metrics.getLoadBalance());
    }

    public RegistrationItem newRegistrationItem(String server) {
        RegistrationItem item = new RegistrationItem();
        item.setPrimary(server);
        return item;
    }
}
