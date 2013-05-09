package org.sipfoundry.sipxconfig.mongo;

import static org.junit.Assert.assertEquals;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;

import com.mongodb.util.JSON;

public class MongoApiTest {
    private MongoApi m_api;
    
    @Before
    public void setUp() {
        Domain d = new Domain("hubler.us");
        d.setNetworkName("hubler.us");
        new DomainManagerImpl().setTestDomain(d);
        m_api = new MongoApi();
        m_api.setMongoManager(new MongoManagerImpl());        
    }

    @Test
    public void testHealthy() throws IOException {
        MongoMeta meta = testData("three-node-healthy");
        Location l1 = new Location("swift.hubler.us");
        Location l2 = new Location("goose.hubler.us");
        List<Location> locations = Arrays.asList(l1, l2);
        String actual = JSON.serialize(m_api.metaMap(meta, locations));        
        String expected = IOUtils.toString(getClass().getResourceAsStream("three-node-healthy.expected.json"));
        assertEqualJson(expected, actual);
    }
    
    void assertEqualJson(String expectedJson, String actualJson) {
        Map<?, ?> e = (Map<?, ?>) JSON.parse(expectedJson);
        ByteArrayOutputStream expected = new ByteArrayOutputStream();
        MapUtils.debugPrint(new PrintStream(expected), "", e);
         
        Map<?, ?> a = (Map<?, ?>) JSON.parse(actualJson);
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        MapUtils.debugPrint(new PrintStream(actual), "", a);
        
        assertEquals(expected.toString(), actual.toString());
    }
    
    @Test
    public void testMissing() throws IOException {
        MongoMeta meta = testData("three-node-missing-arbiter-and-database");
        Location l1 = new Location("swift.hubler.us");
        Location l2 = new Location("goose.hubler.us");
        List<Location> locations = Arrays.asList(l1, l2);
        String actual = JSON.serialize(m_api.metaMap(meta, locations));        
        String expected = IOUtils.toString(getClass().getResourceAsStream("three-node-missing-arbiter-and-database.expected.json"));
        assertEqualJson(expected, actual);
    }

    MongoMeta testData(String id) throws IOException {
        MongoMeta meta = new MongoMeta();
        String status = IOUtils.toString(getClass().getResourceAsStream(id + ".status.json"));
        meta.setStatusToken(status);
        String analysis = IOUtils.toString(getClass().getResourceAsStream(id + ".analysis.json")); 
        meta.setAnalysisToken(analysis);
        return meta;
    }
}
