package org.sipfoundry.commons.mongo;

import static org.junit.Assert.assertEquals;

import java.io.File;

import org.junit.Test;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

import com.mongodb.Mongo;

public class MongoFactoryTest {
    @Test
    public void configFile() {
    	System.out.println(new File(".").getAbsolutePath());
    	System.setProperty("conf.dir", ".");
        String url = UnfortunateLackOfSpringSupportFactory.getConnectionURL();
        assertEquals("mongodb://server1:27017,server2:27018/?slaveOk=true", url);
    }

    @Test
    public void mongoObject() throws Exception {
        Mongo m = MongoFactory.fromConnectionString("mongodb://localhost:27017/?safe=true");
        assertEquals(true, m.getMongoOptions().safe);
        m.getDB("imdb");
    }
}
