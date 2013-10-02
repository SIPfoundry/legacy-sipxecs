package org.sipfoundry.sipxconfig.dns;


import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsApiTest {
    private Region m_r1 = new Region("US");
    private Region m_r2 = new Region("EU");
    private Location m_s1R1 = new Location("server-1-region-1", "1.1.1.1");
    private Location m_s1R2 = new Location("server-1-region-2", "1.1.1.2");
    
    @BeforeClass
    public static void setUpClass() {
        Domain domain = new Domain();
        domain.setName("example.com");
        DomainManagerImpl manager = new DomainManagerImpl();
        manager.setTestDomain(domain);
    }
    
    @Before
    public void setUp() {
        m_r1.setUniqueId(1);
        m_r2.setUniqueId(2);        
        m_s1R1.setUniqueId(11);
        m_s1R1.setRegionId(m_r1.getId());
        m_s1R2.setUniqueId(21);
        m_s1R2.setRegionId(m_r2.getId());
    }
    
    @Test
    public void testReadPlan() throws IllegalAccessException, InvocationTargetException, IOException {
        // we should get all our data back json->object->json
        String seedAndExpectedJsonFile = "plan.json";
        InputStream json = getClass().getResourceAsStream(seedAndExpectedJsonFile);
        DnsApi api = new DnsApi();
        DnsFailoverPlan plan = api.readPlan(json, Arrays.asList(m_r1, m_r2), Arrays.asList(m_s1R1, m_s1R2));
        json.close();
        String expected = IOUtils.toString(getClass().getResourceAsStream(seedAndExpectedJsonFile));
        TestHelper.assertEquals(expected, plan);
    }

    @Test
    public void testServiceCandidates() throws IOException {
        String expected = IOUtils.toString(getClass().getResourceAsStream("service-candidates.expected.json"));
        DnsApi api = new DnsApi();
        StringWriter actual = new StringWriter();
        List<Region> regions = Arrays.asList(m_r1, m_r2);
        List<Location> locations = Arrays.asList(m_s1R1, m_s1R2);        
        api.writeTargetCandidates(actual, regions, locations);
        TestHelper.assertEqualJson2(expected, actual.toString());
    }

    @Test
    public void testPlan() throws IOException {
        DnsApi api = new DnsApi();
        DnsFailoverPlan plan = new DnsFailoverPlan();
        StringWriter actual = new StringWriter();
        api.writePlan(actual, plan);
    }
}
