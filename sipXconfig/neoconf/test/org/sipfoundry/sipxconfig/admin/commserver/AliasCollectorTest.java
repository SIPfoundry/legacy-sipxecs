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
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

public class AliasCollectorTest extends TestCase {

    public void testGetAliasMappings() {
        final int len = 10;
        final IMocksControl[] apCtrl = new IMocksControl[len];
        final AliasProvider[] ap = new AliasProvider[len];
        final IMocksControl[] apCtrl2 = new IMocksControl[len];
        final Replicable[] entity = new Replicable[len];
        

        AliasMapping alias = new AliasMapping();
        
        for (int i = 0; i < len; i++) {
            apCtrl[i] = EasyMock.createControl();
            ap[i] = apCtrl[i].createMock(AliasProvider.class);
            ap[i].getAliasMappings();
            apCtrl2[i] = EasyMock.createControl();
            entity[i] = apCtrl2[i].createMock(Replicable.class); 
            apCtrl[i].andReturn(Collections.singletonMap(entity[i], Collections.singletonList(alias))).anyTimes();
            apCtrl[i].replay();
        }

        AliasCollector collector = new AliasCollector() {
            protected Collection getAliasProviders() {
                return Arrays.asList(ap);
            }
        };

        Map<Replicable, Collection<AliasMapping>> aliases = collector.getAliasMappings();
        assertEquals(len, aliases.size());
        for (int i=0; i<10; i++ ) {
            List<AliasMapping> aliasMappings = (List<AliasMapping>) aliases.get(entity[i]);
            for (Iterator iter = aliasMappings.iterator(); iter.hasNext();) {
                assertSame(alias, iter.next());
            }    
        }
        

        for (int i = 0; i < len; i++) {
            apCtrl[i].verify();
        }
    }
}
