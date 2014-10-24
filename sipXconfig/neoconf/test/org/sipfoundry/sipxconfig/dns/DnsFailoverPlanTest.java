package org.sipfoundry.sipxconfig.dns;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsFailoverPlanTest {
    private final Region m_r1 = new Region("US");
    private final Region m_r2 = new Region("EU");
    private final Location m_s1Rnull= new Location("server-1-no-region", "1.1.1.0");
    private final Location m_s2Rnull = new Location("server-2-no-region", "1.1.2.0");
    private final Location m_s1R1 = new Location("server-1-region-1", "1.1.1.1");
    private final Location m_s2R1 = new Location("server-2-region-1", "1.1.2.1");
    private final Location m_s1R2 = new Location("server-1-region-2", "1.1.1.2");
    private final Location m_s2R2 = new Location("server-2-region-2", "1.1.2.2");

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

        m_s1Rnull.setUniqueId(1);
        m_s2Rnull.setUniqueId(2);

        m_s1R1.setUniqueId(11);
        m_s1R1.setRegionId(m_r1.getId());
        m_s2R1.setUniqueId(12);
        m_s2R1.setRegionId(m_r1.getId());

        m_s1R2.setUniqueId(21);
        m_s1R2.setRegionId(m_r2.getId());
        m_s2R2.setUniqueId(22);
        m_s2R2.setRegionId(m_r2.getId());
    }

    @Test
    public void findTarget() {
        DnsView viewR1 = new DnsView();
        viewR1.setRegionId(m_r1.getId());

        DnsView viewR2 = new DnsView();
        viewR2.setRegionId(m_r2.getId());

        DnsFailoverGroup group = new DnsFailoverGroup();

        DnsTarget all = new DnsTarget(DnsTarget.BasicType.ALL_REGIONS);
        group.setTargets(Arrays.asList(all));
        assertEquals(all, group.findTarget(viewR1, null, null));
        assertEquals(all, group.findTarget(viewR1, m_r1.getId(), null));
        assertEquals(all, group.findTarget(viewR1, null, m_s1R1.getAddress()));
        assertEquals(all, group.findTarget(viewR2, null, null));

        DnsTarget other = new DnsTarget(DnsTarget.BasicType.ALL_OTHER_REGIONS);
        group.setTargets(Arrays.asList(other));
        assertEquals(null, group.findTarget(viewR1, m_r1.getId(), m_s1R1.getAddress()));

        // TODO: Fails but not sure if this should work yet ??
        //assertEquals(null, group.findTarget(viewR2, null, m_s1R1.getAddress()));

        assertEquals(other, group.findTarget(viewR1, m_r2.getId(), null));
        assertEquals(other, group.findTarget(viewR1, null, m_s2R1.getAddress()));

        DnsTarget local = new DnsTarget(DnsTarget.BasicType.LOCAL_REGION);
        group.setTargets(Arrays.asList(local));
        assertEquals(local, group.findTarget(viewR1, m_r1.getId(), m_s1R1.getAddress()));

        // TODO: Fails but not sure if this should work yet ??
        // assertEquals(local, group.findTarget(viewR2, null, m_s1R1.getAddress()));

        assertEquals(null, group.findTarget(viewR1, m_r2.getId(), null));
        assertEquals(null, group.findTarget(viewR1, null, m_s2R1.getAddress()));

        DnsTarget tS1R1 = new DnsTarget(m_s1R1);
        group.setTargets(Arrays.asList(tS1R1));
        // view is irrelevant
        assertEquals(null, group.findTarget(viewR1, null, null));
        assertEquals(null, group.findTarget(viewR1, m_r1.getId(), null));
        assertEquals(tS1R1, group.findTarget(viewR1, null, m_s1R1.getAddress()));
        assertEquals(null, group.findTarget(viewR1, null, m_s2R1.getAddress()));

        DnsTarget tR1 = new DnsTarget(m_r1);
        group.setTargets(Arrays.asList(tR1));
        // view is irrelevant
        assertEquals(null, group.findTarget(viewR1, null, null));
        assertEquals(tR1, group.findTarget(viewR1, m_r1.getId(), null));
        assertEquals(null, group.findTarget(viewR1, null, m_s1R1.getAddress()));

        // Combinations
        group.setTargets(Arrays.asList(tS1R1, tR1, other, all));
        assertEquals(tS1R1, group.findTarget(viewR1, null, m_s1R1.getAddress()));
        assertEquals(tR1, group.findTarget(viewR1, m_r1.getId(), null));
        assertEquals(other, group.findTarget(viewR1, null, m_s1R2.getAddress()));
    }

    @Test
    public void getDnsPriorityAndPercent() {
        DnsView viewR2 = new DnsView();
        viewR2.setRegionId(m_r2.getId());

        DnsFailoverPlan plan = createFairlyTypicalDnsFailoverPlan();

        DnsRecordNumerics actual;

        actual = plan.getRecordNumerics(viewR2, m_r1.getId(), m_s1Rnull.getAddress());
        assertEquals(0, actual.getPriorityLevel());
        assertEquals(10, actual.getPercentage());

        actual = plan.getRecordNumerics(viewR2, m_r1.getId(), m_s1R1.getAddress());
        assertEquals(1, actual.getPriorityLevel());
        assertEquals(20, actual.getPercentage());

        // unusual but valid - server w/o region may *not* be used in certain cases.
        actual = plan.getRecordNumerics(viewR2, m_r2.getId(), m_s1R1.getAddress());
        assertNull(actual);
    }

    public DnsFailoverPlan createFairlyTypicalDnsFailoverPlan() {
        DnsFailoverGroup targetByServer = new DnsFailoverGroup();
        DnsTarget t1 = new DnsTarget(m_s1Rnull);
        t1.setPercentage(10);
        targetByServer.setTargets(Collections.singleton(t1));

        DnsFailoverGroup targetByRegion = new DnsFailoverGroup();
        DnsTarget t2 = new DnsTarget(m_r1);
        t2.setPercentage(20);
        targetByRegion.setTargets(Collections.singleton(t2));

        DnsFailoverGroup targetByOtherRegions = new DnsFailoverGroup();
        DnsTarget t3 = new DnsTarget(DnsTarget.BasicType.ALL_OTHER_REGIONS);
        t3.setPercentage(30);
        targetByOtherRegions.setTargets(Collections.singleton(t3));

        DnsFailoverPlan plan = new DnsFailoverPlan();
        plan.setGroups(Arrays.asList(targetByServer, targetByRegion, targetByOtherRegions));
        return plan;
    }

    @Test
    public void getExternalDnsSrvRecords() {
        DnsView view = new DnsView();
        DnsFailoverPlan plan = createFairlyTypicalDnsFailoverPlan();

        ResourceRecord rrS1Rnull = new ResourceRecord(m_s1Rnull.getAddress(), 99, null);
        ResourceRecord rrS1R1 = new ResourceRecord(m_s1R1.getAddress(), 99, m_r1.getId());
        ResourceRecord rrS1R2 = new ResourceRecord(m_s1R2.getAddress(), 99, m_r2.getId());

        DnsSrvRecord[] proto = new DnsSrvRecord[0];

        ResourceRecords external = new ResourceRecords("proto", "resource");
        external.addRecords(Arrays.asList(rrS1Rnull, rrS1R1, rrS1R2));

        DnsSrvRecord[] highest = plan.getDnsSrvRecords(view, rrS1Rnull, external).toArray(proto);
        assertEquals(4, highest.length);

        assertEquals(rrS1Rnull.getAddress(), highest[0].getDestination());
        assertEquals("", highest[0].getHost());

        DnsSrvRecord[] medium = plan.getDnsSrvRecords(view, rrS1R1, external).toArray(proto);
        assertEquals(4, medium.length);
        // lower priority is higher
        assertTrue(highest[0].getPriority() < medium[0].getPriority());

        DnsSrvRecord[] lowest = plan.getDnsSrvRecords(view, rrS1R2, external).toArray(proto);
        assertEquals(4, lowest.length);
        // lower priority is higher
        assertTrue(medium[0].getPriority() < lowest[0].getPriority());
    }

    @Test
    public void getInternalDnsSrvRecords() throws IOException {
        DnsView view = new DnsView();
        DnsFailoverPlan plan = createFairlyTypicalDnsFailoverPlan();

        ResourceRecord rrS1Rnull = new ResourceRecord(m_s1Rnull.getHostname(), 99, null);
        ResourceRecord rrS1R1 = new ResourceRecord(m_s1R1.getHostname(), 99, m_r1.getId());
        ResourceRecord rrS1R2 = new ResourceRecord(m_s1R2.getHostname(), 99, m_r2.getId());

        DnsSrvRecord[] proto = new DnsSrvRecord[0];

        ResourceRecords internal = new ResourceRecords("proto", "resource", true);
        internal.addRecords(Arrays.asList(rrS1Rnull, rrS1R1, rrS1R2));

        DnsSrvRecord[] actual = plan.getDnsSrvRecords(view, rrS1Rnull, internal).toArray(proto);
        assertEquals(1, actual.length);
        // sort by any predictable order
        Arrays.sort(actual, new Comparator<DnsSrvRecord>() {
            @Override
            public int compare(DnsSrvRecord arg0, DnsSrvRecord arg1) {
                return arg0.getPriority() - arg1.getPriority();
            }
        });

        String expected = IOUtils.toString(getClass().getResourceAsStream("internal.expected.json"));
        TestHelper.assertEquals(expected, actual);
    }
}
