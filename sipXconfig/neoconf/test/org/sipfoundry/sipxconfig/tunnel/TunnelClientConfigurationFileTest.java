package org.sipfoundry.sipxconfig.tunnel;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.InputStream;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.tunnel.TunnelClientConfigurationFile;

public class TunnelClientConfigurationFileTest extends TestCase {
    
	public void testConfig() throws Exception {
		TunnelClientConfigurationFile f = new TunnelClientConfigurationFile();
		f.setTemplate("tunnel/tunnel-client.conf.vm");
		f.setVelocityEngine(TestHelper.getVelocityEngine());
        
        Location l1 = new Location();
        l1.setFqdn("primary.example.com");
        l1.setPrimary(true);

        Location l2 = new Location();
        l2.setFqdn("secondary1.example.com");

        Location l3 = new Location();
        l3.setFqdn("secondary2.example.com");

        // DNS generator calls getLocations three times each time it's called.
        LocationsManager lm = createMock(LocationsManager.class);
        lm.getPrimaryLocation();
        expectLastCall().andReturn(l1).anyTimes();
        lm.getLocations();
        expectLastCall().andReturn(array(l1, l2, l3)).anyTimes();
       
        replay(lm);
        
		f.setLocationsManager(lm);
		StringWriter actual = new StringWriter();
		f.write(actual, l1);
		
        InputStream expected = getClass().getResourceAsStream("stunnel-client.conf");
        assertEquals(IOUtils.toString(expected), actual.toString());
		
		verify(lm);
	}

	private static <T> T[] array(T... items) {
        return items;
    }
}
