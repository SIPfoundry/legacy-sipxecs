/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

import java.util.HashSet;
import java.util.Random;

import org.sipfoundry.sipxrelay.Parity;
import org.sipfoundry.sipxrelay.PortRange;
import org.sipfoundry.sipxrelay.PortRangeManager;

import junit.framework.TestCase;

public class PortRangeManagerTest extends TestCase {
    
    PortRangeManager portRangeManager;
    
    public void setUp() {
        portRangeManager = new PortRangeManager(25000,1000000);
        
    }
    
    public void testAllocationFree() {
        PortRange portRange1 = portRangeManager.allocate(100, Parity.EVEN);
        assertEquals("Must be 100", portRange1.range(), 100);
        assertTrue("Must be even boundary",portRange1.getLowerBound()%2 == 0);
        portRangeManager.free(portRange1);
        assertEquals(portRangeManager.getRangeCount() , 1);
        HashSet<PortRange> portRanges = new HashSet<PortRange>();
        
        for ( int i = 0 ; i < 100; i ++ ) {
            
            int size = Math.abs(new Random().nextInt() % 100 ) + 2;
            PortRange portRange = portRangeManager.allocate(size, i %2 == 0 ? Parity.EVEN : Parity.ODD);
            assertNotNull (portRange);
            portRanges.add(portRange);
        }
        
        for ( PortRange portRange : portRanges) {
            portRangeManager.free(portRange);
        }
        assertEquals(portRangeManager.getRangeCount(),1);
        PortRange p1 = portRangeManager.allocate(1, Parity.EVEN);
        PortRange p2 = portRangeManager.allocate(1, Parity.EVEN);
        portRangeManager.free(p1);
        portRangeManager.free(p2);
        PortRange p3 = portRangeManager.allocate(1, Parity.EVEN);
        assertEquals("Should re-allocate at the same point in the range", p1.getLowerBound(),p3.getLowerBound());
        assertEquals("Should allocate one port", p3.getLowerBound() + 1, p3.getHigherBound());
        portRangeManager.free(p3);
        
        PortRange p4 = portRangeManager.allocate(2, Parity.EVEN);
        
        portRangeManager.free( new PortRange( p4.getLowerBound(),
                p4.getLowerBound() + 1));
        portRangeManager.free( new PortRange(p4.getLowerBound() + 1, p4.getLowerBound() + 2 ));
        
        PortRange p5 = portRangeManager.allocate(1, Parity.EVEN);
        assertEquals(p1.getLowerBound() , p5.getLowerBound());
        
        
        
    }

}
