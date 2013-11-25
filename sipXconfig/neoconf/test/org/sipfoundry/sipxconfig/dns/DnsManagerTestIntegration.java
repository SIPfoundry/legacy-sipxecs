package org.sipfoundry.sipxconfig.dns;

import static org.junit.Assert.assertArrayEquals;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.dns.DnsTarget.BasicType;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsManagerTestIntegration extends IntegrationTestCase {
    private DnsManager m_dnsManager;
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("dns/seed-plan.sql");
    }

    public void testReadPlans() throws IOException {        
        Collection<DnsFailoverPlan> actual = m_dnsManager.getPlans();
        String expected = IOUtils.toString(getClass().getResourceAsStream("plans.expected.json"));
        TestHelper.assertEquals(expected, actual);
    }

    public void testReadPlan() throws IOException {        
        DnsFailoverPlan actual = m_dnsManager.getPlan(1);
        assertNotNull(actual);
        assertEquals(1, (int) actual.getId());
    }

    public void testWritePlan() throws IOException {
        DnsFailoverPlan plan = new DnsFailoverPlan();
        plan.setName("test");
        DnsFailoverGroup g1 = new DnsFailoverGroup();
        DnsFailoverGroup g2 = new DnsFailoverGroup();
        DnsFailoverGroup g3 = new DnsFailoverGroup();
        List<DnsFailoverGroup> groups = Arrays.asList(g1, g2, g3);
        Region r1 = m_regionManager.getRegion(1);
        DnsTarget g1t1 = new DnsTarget(r1);
        g1.setTargets(Collections.singleton(g1t1));
        Location l1 = m_locationsManager.getLocation(1);
        Location l2 = m_locationsManager.getLocation(2);
        DnsTarget g2t1 = new DnsTarget(l1);
        g2t1.setPercentage(50);
        DnsTarget g2t2 = new DnsTarget(l2);
        g2t2.setPercentage(50);
        g2.setTargets(Arrays.asList(g2t1, g2t2));        
        DnsTarget g3t1 = new DnsTarget(BasicType.LOCAL_REGION);
        DnsTarget g3t2 = new DnsTarget(BasicType.ALL_OTHER_REGIONS);
        g3.setTargets(Arrays.asList(g3t1, g3t2));                
        plan.setGroups(groups);
        m_dnsManager.savePlan(plan);
    }
    
    public void testGetViews() {
        Collection<DnsView> views = m_dnsManager.getViews();
        assertEquals(3, views.size());
        
        DnsView[] actual = views.toArray(new DnsView[0]);
        actual[0].setName("ZZZ");
        m_dnsManager.saveView(actual[0]);
        
        DnsView another = new DnsView();
        another.setPlanId(1);
        another.setRegionId(1);
        another.setCustomRecordsIds(Arrays.asList(100, 101));
        another.setName("another");
        m_dnsManager.saveView(another);
        Collection<DnsView> views2 = m_dnsManager.getViews();
        assertEquals(4, views2.size());
        
        DnsView reread = m_dnsManager.getViewById(another.getId());
        assertEquals(2, reread.getCustomRecordsIds().size());
    }
    
    public void testMoveById() {
        // Before
        //   1, v1-p1-r1
        //   2, v2-p1-r1-off
        //   3, v3-p2-r2
        // After
        //   2, v2-p1-r1-off
        //   3, v3-p2-r2
        //   1, v1-p1-r1
        DnsView[] before = m_dnsManager.getViews().toArray(new DnsView[0]);
        m_dnsManager.moveViewById(new Integer[] {2, 3}, -1);
        DnsView[] after = m_dnsManager.getViews().toArray(new DnsView[0]);
        assertEquals(before[1].getId(), after[0].getId());
        assertEquals(before[2].getId(), after[1].getId());
        assertEquals(before[0].getId(), after[2].getId());
    }
    
    public void testInUse() {
        String[] actual;
        Region r1 = m_regionManager.getRegion(1);
        actual = m_dnsManager.getPlanNamesUsingRegion(r1);
        assertArrayEquals(new String[] {"one region"}, actual);
        actual = m_dnsManager.getViewNamesUsingRegion(r1);
        assertArrayEquals(new String[] {"v1-p1-r1", "v2-p1-r1-off"}, actual);
        Location l1 = m_locationsManager.getLocation(1);
        actual = m_dnsManager.getPlanNamesUsingLocation(l1);
        assertArrayEquals(new String[] {"typical", "four servers unequally"}, actual);
        DnsFailoverPlan plan = m_dnsManager.getPlan(2);
        actual = m_dnsManager.getViewNamesUsingPlan(plan);
        assertArrayEquals(new String[] {"v3-p2-r2"}, actual);
    }
    
    public void testCustom() throws IOException {
        Collection<DnsCustomRecords> actual = m_dnsManager.getCustomRecords();
        String expected = IOUtils.toString(getClass().getResourceAsStream("customs.expected.json"));
        TestHelper.assertEquals(expected, actual);
    }
    
    public void testCustomSave() throws IOException {
        DnsCustomRecords records = new DnsCustomRecords();
        records.setName("x");
        records.setRecords("x IN A 10.1.1.1");
        m_dnsManager.saveCustomRecords(records);
        Integer id = records.getId();
        DnsCustomRecords after = m_dnsManager.getCustomRecordsById(id);
        assertEquals(after.getName(), records.getName());
        m_dnsManager.deleteCustomRecords(after);
        assertNull(m_dnsManager.getCustomRecordsById(id));        
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
