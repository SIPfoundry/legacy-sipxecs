package org.sipfoundry.commons.mongo;

import static org.junit.Assert.assertArrayEquals;

import java.util.List;

import org.junit.Test;

import com.mongodb.MongoURI;


public class MongoDbTemplateTest {
    
    @Test
    public void parseGood() {
        String file = MongoDbTemplateTest.class.getResource("mongo-client-sample.ini").getFile();
        MongoURI uri = MongoDbTemplate.parseFromFile(file);
        List<String> actualHosts = uri.getHosts();
        assertArrayEquals(new Object[] { "server1:27017", "server2:27018" }, actualHosts.toArray());
    }
}
