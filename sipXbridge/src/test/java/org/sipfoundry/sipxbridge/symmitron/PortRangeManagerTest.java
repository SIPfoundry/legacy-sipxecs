package org.sipfoundry.sipxbridge.symmitron;

import java.util.HashSet;
import java.util.Random;

import org.sipfoundry.sipxbridge.symmitron.Parity;
import org.sipfoundry.sipxbridge.symmitron.PortRange;
import org.sipfoundry.sipxbridge.symmitron.PortRangeManager;

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
    }

}
