package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;
import java.io.IOException;
import java.io.StringWriter;

import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;


public class MongoClientConfigTest {
    
    private final String SINGLE_SETUP = "# example sipxecs/localhost:27017,localhost:27018.  Mostly for C++ driver\n"
            + "connectionString = sipxecs/localhost:27017\n"
            + "# example mongodb://localhost:27017,localhost:27018?replset=sipxecs.  Used on java and ruby drivers.\n"
            + "connectionUrl = mongodb://localhost:27017?replset=sipxecs\n";
    
    @Test
    public void singleServerConfig() throws IOException {
        Location location = null;
        StringWriter actual = new StringWriter();
        MongoClientConfig config = new MongoClientConfig();
        config.setVelocityEngine(TestHelper.getVelocityEngine());
        config.write(actual, location);
        assertEquals(SINGLE_SETUP, actual.toString());
    }
}
