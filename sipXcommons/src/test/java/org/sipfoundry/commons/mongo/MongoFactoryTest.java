package org.sipfoundry.commons.mongo;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import com.mongodb.Mongo;

public class MongoFactoryTest {
    @Test
    public void configFile() {
        String file = MongoFactoryTest.class.getResource("mongo-client-sample.ini").getFile();
        String url = MongoFactory.readConfig(file);
        assertEquals("mongodb://server1:27017,server2:27018/?slaveOk=true", url);
    }

    @Test
    public void mongoObject() throws Exception {
        Mongo m = MongoFactory.fromConnectionString("mongodb://localhost:27017/?safe=true");
        assertEquals(true, m.getMongoOptions().safe);
        m.getDB("imdb");
    }
}
