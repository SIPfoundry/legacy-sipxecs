package org.sipfoundry.commons.mongo;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import org.junit.Test;
import org.springframework.data.mongodb.core.MongoFactoryBean;

public class MongoFactoryTest {
    @Test
    public void configFile() {
        String file = MongoFactoryTest.class.getResource("mongo-client-sample.ini").getFile();
        MongoFactory f = new MongoFactory();
        f.setConfigFile(file);
        f.readConfig();
        assertEquals("mongodb://server1:27017,server2:27018/?slaveOk=true", f.getConnectionUrl());
    }

    @Test
    public void mongoObject() throws Exception {
        MongoFactory f = new MongoFactory();
        f.setConnectionUrl("mongodb://localhost:27017/?safe=true");
        MongoFactoryBean mongo = f.mongo();
        assertEquals(true, mongo.getObject().getMongoOptions().safe);
        assertNotNull(mongo);
    }
}
