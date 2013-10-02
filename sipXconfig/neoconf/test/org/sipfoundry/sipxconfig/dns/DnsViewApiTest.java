package org.sipfoundry.sipxconfig.dns;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsViewApiTest {
    private Region m_r1 = new Region("US");
    private Region m_r2 = new Region("EU");
    private DnsFailoverPlan m_p1 = new DnsFailoverPlan("p1");
    private DnsFailoverPlan m_p2 = new DnsFailoverPlan("p2");
    
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
        m_p1.setUniqueId(10);
        m_p2.setUniqueId(20);
    }

    private DnsViewApi m_dnsViewApi;

    @Test
    public void writeView() throws IOException {
        String expected = IOUtils.toString(getClass().getResourceAsStream("view.expected.json"));
        StringWriter actual = new StringWriter();
        DnsView view = new DnsView();
        view.setName("x");
        view.setRegionId(1);
        view.setPlanId(2);
        m_dnsViewApi = new DnsViewApi();
        Collection<Region> regions = Collections.singleton(m_r1);
        Collection<DnsFailoverPlan> plans = Collections.singleton(m_p1);
        m_dnsViewApi.writeView(actual, view, plans, regions);
        TestHelper.assertEqualJson2(expected, actual.toString());
    }
    
    @Test
    public void writeViews() throws IOException {
        DnsView v1 = new DnsView();
        v1.setName("x");
        v1.setRegionId(m_r1.getId());
        v1.setPlanId(m_p1.getId());
        DnsView v2 = new DnsView();
        v2.setName("y");
        v2.setRegionId(m_r2.getId());
        v2.setPlanId(m_p2.getId());
        StringWriter actual = new StringWriter();
        m_dnsViewApi = new DnsViewApi();
        Collection<Region> regions = Arrays.asList(m_r1, m_r2);
        Collection<DnsFailoverPlan> plans = Arrays.asList(m_p1, m_p2);
        Collection<DnsView> views = Arrays.asList(v1, v2);
        m_dnsViewApi.writeViews(actual, views, regions, plans);
        String expected = IOUtils.toString(getClass().getResourceAsStream("views.expected.json"));
        TestHelper.assertEqualJson2(expected, actual.toString());
    }
}
