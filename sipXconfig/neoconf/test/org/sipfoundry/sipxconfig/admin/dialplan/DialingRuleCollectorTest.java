/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class DialingRuleCollectorTest extends TestCase {

    public void testGetDialingRuleProviders() {
        final int len = 10;
        final IMocksControl[] drpCtrl = new IMocksControl[len];
        final DialingRuleProvider[] drp = new DialingRuleProvider[len];

        // create a dummy dialing rule and put it in a list
        DialingRule rule = new IntercomRule(true, "*666", "intercomCode", 42);
        List<DialingRule> rules = new ArrayList<DialingRule>();
        rules.add(rule);

        for (int i = 0; i < len; i++) {
            drpCtrl[i] = EasyMock.createControl();
            drp[i] = drpCtrl[i].createMock(DialingRuleProvider.class);
            drp[i].getDialingRules();
            drpCtrl[i].andReturn(rules).anyTimes();
            drpCtrl[i].replay();
        }

        DialingRuleCollector collector = new DialingRuleCollector() {
            protected Collection getDialingRuleProviders() {
                return Arrays.asList(drp);
            }
        };

        rules = collector.getDialingRules();
        assertEquals(len, rules.size());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            assertSame(rule, i.next());
        }

        for (int i = 0; i < len; i++) {
            drpCtrl[i].verify();
        }
    }

    public void testGetDialingRuleProvidersDisabled() {
        final int len = 10;
        final IMocksControl[] drpCtrl = new IMocksControl[len];
        final DialingRuleProvider[] drp = new DialingRuleProvider[len];

        // create a dummy dialing rule and put it in a list
        DialingRule rule = new IntercomRule(false, "*666", "intercomCode", 42);
        List<DialingRule> rules = new ArrayList<DialingRule>();
        rules.add(rule);

        for (int i = 0; i < len; i++) {
            drpCtrl[i] = EasyMock.createControl();
            drp[i] = drpCtrl[i].createMock(DialingRuleProvider.class);
            drp[i].getDialingRules();
            drpCtrl[i].andReturn(rules).anyTimes();
            drpCtrl[i].replay();
        }

        DialingRuleCollector collector = new DialingRuleCollector() {
            protected Collection getDialingRuleProviders() {
                return Arrays.asList(drp);
            }
        };

        rules = collector.getDialingRules();
        // list must be empry since none of the rules were enabled
        assertEquals(0, rules.size());

        for (int i = 0; i < len; i++) {
            drpCtrl[i].verify();
        }
    }

}
