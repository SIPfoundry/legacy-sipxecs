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

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class AliasCollectorTest extends TestCase {

    public void testGetAliasMappings() {
        final int len = 10;
        final IMocksControl[] apCtrl = new IMocksControl[len];
        final AliasProvider[] ap = new AliasProvider[len];

        AliasMapping alias = new AliasMapping();

        for (int i = 0; i < len; i++) {
            apCtrl[i] = EasyMock.createControl();
            ap[i] = apCtrl[i].createMock(AliasProvider.class);
            ap[i].getAliasMappings();
            apCtrl[i].andReturn(Collections.singleton(alias)).anyTimes();
            apCtrl[i].replay();
        }

        AliasCollector collector = new AliasCollector() {
            protected Collection getAliasProviders() {
                return Arrays.asList(ap);
            }
        };

        Collection aliasMappings = collector.getAliasMappings();
        assertEquals(len, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            assertSame(alias, i.next());
        }

        for (int i = 0; i < len; i++) {
            apCtrl[i].verify();
        }
    }
}
