/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.dns;


import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.region.Region;

public class DnsFailoverPlan {
    private DnsFailoverStrategy m_externalPreferSameReqionEqually;
    private DnsFailoverStrategy m_internalPreferLocalServerThenRegionThenOthers;
    private Region m_region;

    DnsFailoverPlan(Region region) {
        m_region = region;
        m_externalPreferSameReqionEqually = new DnsFailoverStrategy() {

            @Override
            public Collection<DnsSrvRecord> getDnsSrvRecords(DnsFailoverPlan plan, ResourceRecord rr,
                    ResourceRecords rrs) {
                DnsSrvRecord record = DnsSrvRecord.domainLevel(rrs.getProto(), rrs.getResource(), rr.getPort(),
                        rr.getAddress());
                record.setWeight(10);
                if (plan.isLocalRegion(rr.getRegionId())) {
                    record.setPriority(10);
                } else {
                    record.setPriority(20);
                }
                return Collections.singleton(record);
            }
        };

        m_internalPreferLocalServerThenRegionThenOthers = new DnsFailoverStrategy() {

            @Override
            public Collection<DnsSrvRecord> getDnsSrvRecords(DnsFailoverPlan plan, ResourceRecord rr,
                    ResourceRecords rrs) {
                List<DnsSrvRecord> srvs = new ArrayList<DnsSrvRecord>();
                DnsSrvRecord srv = DnsSrvRecord.domainLevel(rrs.getProto(), rrs.getResource(), rr.getPort(),
                        rr.getAddress());
                srv.setWeight(10);
                if (plan.isLocalRegion(rr.getRegionId())) {
                    srv.setPriority(10);
                    for (ResourceRecord other : rrs.getRecords()) {
                        DnsSrvRecord rrSrv = DnsSrvRecord.hostLevel(rrs.getProto(), rrs.getResource(),
                                rr.getAddress(), other.getPort(), other.getAddress());
                        rrSrv.setWeight(10);
                        if (other == rr) {
                            rrSrv.setPriority(10);
                        } else if (plan.isLocalRegion(other.getRegionId())) {
                            rrSrv.setPriority(20);
                        } else {
                            rrSrv.setPriority(30);
                        }
                        srvs.add(rrSrv);
                    }
                } else {
                    srv.setPriority(20);
                }

                srvs.add(srv);
                return srvs;
            }
        };
    }

    public boolean isLocalRegion(Integer regionId) {
        if (m_region == null) {
            return (regionId == null);
        }
        return m_region.getId().equals(regionId);
    }

    DnsFailoverStrategy getFailoverStrategy(ResourceRecords rrs) {
        return rrs.isInternal() ? m_internalPreferLocalServerThenRegionThenOthers
                : m_externalPreferSameReqionEqually;
    }
}

interface DnsFailoverStrategy {
    public Collection<DnsSrvRecord> getDnsSrvRecords(DnsFailoverPlan plan, ResourceRecord rr,
            ResourceRecords rrs);
}
