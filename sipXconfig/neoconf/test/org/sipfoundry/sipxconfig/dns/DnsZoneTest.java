package org.sipfoundry.sipxconfig.dns;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.codehaus.jackson.JsonGenerationException;
import org.codehaus.jackson.map.JsonMappingException;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManagerImpl;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class DnsZoneTest {
    private final DnsManagerImpl m_manager = new DnsManagerImpl();
    private final Region m_r1 = new Region("r1");
    private final Region m_r2 = new Region("r2");
    private final Location m_l1 = new Location("l1", "1.1.1.1");
    private final Location m_l2 = new Location("l2", "2.2.2.2");
    private final Location m_l3 = new Location("l3", "3.3.3.3");

    @Before
    public void setUp() {
        new DomainManagerImpl().setTestDomain(new Domain("d1"));
        m_r1.setUniqueId(1);
        m_r2.setUniqueId(2);
        m_l1.setRegionId(m_r1.getId());
        m_l2.setRegionId(m_r1.getId());
        m_l3.setRegionId(m_r2.getId());
    }

    @Test
    public void external() throws JsonGenerationException, JsonMappingException, IOException {
        final ResourceRecords records = new ResourceRecords("_x", "test", false);
        records.addRecord(new ResourceRecord(m_l1.getHostname(), 1, m_l1.getRegionId()));
        records.addRecord(new ResourceRecord(m_l2.getHostname(), 2, m_l2.getRegionId()));
        records.addRecord(new ResourceRecord(m_l3.getHostname(), 3, m_l3.getRegionId()));
        DnsProvider p = new DnsProvider() {
            @Override
            public Collection<ResourceRecords> getResourceRecords(DnsManager manager) {
                return Collections.singleton(records);
            }

            @Override
            public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses,
                    Location whoIsAsking) {
                // TODO Auto-generated method stub
                return null;
            }
        };
        m_manager.setProviders(Collections.singletonList(p));
        DnsView v = new DnsView(m_r1.getName());
        v.setRegionId(m_r1.getId());
        v.setPlanId(DnsFailoverPlan.FALLBACK);
        Collection<DnsSrvRecord> actual = m_manager.getResourceRecords(v);
        String expected = IOUtils.toString(getClass().getResourceAsStream("external-zone.expected.json"));
        TestHelper.assertEquals(expected, actual);
    }


    @Test
    public void internal() throws JsonGenerationException, JsonMappingException, IOException {
        final ResourceRecords records = new ResourceRecords("_y", "test", true);
        records.addRecord(new ResourceRecord(m_l1.getHostname(), 1, m_l1.getRegionId()));
        records.addRecord(new ResourceRecord(m_l2.getHostname(), 2, m_l2.getRegionId()));
        records.addRecord(new ResourceRecord(m_l3.getHostname(), 3, m_l3.getRegionId()));
        DnsProvider p = new DnsProvider() {
            @Override
            public Collection<ResourceRecords> getResourceRecords(DnsManager manager) {
                return Collections.singleton(records);
            }

            @Override
            public Address getAddress(DnsManager manager, AddressType t, Collection<Address> addresses,
                    Location whoIsAsking) {
                // TODO Auto-generated method stub
                return null;
            }
        };
        m_manager.setProviders(Collections.singletonList(p));
        DnsView v = new DnsView(m_r1.getName());
        v.setRegionId(m_r1.getId());
        v.setPlanId(DnsFailoverPlan.FALLBACK);
        Collection<DnsSrvRecord> actual = m_manager.getResourceRecords(v);
        String expected = IOUtils.toString(getClass().getResourceAsStream("internal-zone.expected.json"));
        TestHelper.assertEquals(expected, actual);
    }
}
