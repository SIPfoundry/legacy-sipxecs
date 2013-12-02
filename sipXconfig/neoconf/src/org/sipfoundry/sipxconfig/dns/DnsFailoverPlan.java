/**
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
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.codehaus.jackson.annotate.JsonIgnore;
import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.feature.Feature;

@JsonPropertyOrder(alphabetic = true)
public class DnsFailoverPlan extends BeanWithId implements NamedObject, DeployConfigOnEdit {
    private String m_name;
    private Collection<DnsFailoverGroup> m_groups;

    public DnsFailoverPlan() {
    }

    public DnsFailoverPlan(String name) {
        setName(name);
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public Collection<DnsFailoverGroup> getGroups() {
        return m_groups;
    }

    public void setGroups(Collection<DnsFailoverGroup> groups) {
        m_groups = groups;
    }

    DnsRecordNumerics getRecordNumerics(DnsView view, Integer regionId, String address) {
        int priorityLevel = 0;
        if (m_groups != null) {
            for (DnsFailoverGroup group : m_groups) {
                DnsTarget target = group.findTarget(view, regionId, address);
                if (target != null) {
                    int percentage = target.getPercentage();
                    return new DnsRecordNumerics(priorityLevel, percentage);
                }
                priorityLevel++;
            }
        }
        return null;
    }

    Collection<DnsSrvRecord> getDnsSrvRecords(DnsView view, ResourceRecord rr, ResourceRecords rrs) {
        DnsRecordNumerics prioAndPct = getRecordNumerics(view, rr.getRegionId(), rr.getAddress());
        List<DnsSrvRecord> srvs = new ArrayList<DnsSrvRecord>();
        if (rrs.isInternal()) {
            if (prioAndPct != null) {
                DnsSrvRecord srv = DnsSrvRecord.domainLevel(rrs.getProto(), rrs.getResource(), rr.getPort(),
                        rr.getAddress());
                srv.setWeight(prioAndPct.getWeight());
                srv.setPriority(prioAndPct.getPriority());
                srv.setInternal(true);
                srvs.add(srv);
            }
            for (ResourceRecord other : rrs.getRecords()) {
                DnsSrvRecord rrSrv = DnsSrvRecord.hostLevel(rrs.getProto(), rrs.getResource(), rr.getAddress(),
                        other.getPort(), other.getAddress());
                rrSrv.setInternal(true);
                if (other == rr) {
                    // SRV records that points local services to other local services
                    rrSrv.setPriority(DnsRecordNumerics.HIGHEST_PRIORITY);
                    // weight is irrelevant when only 1 record to consider
                    rrSrv.setWeight(DnsRecordNumerics.INCONSEQUENTIAL_PERCENTAGE);
                    srvs.add(rrSrv);
                } else {
                    DnsRecordNumerics otherPrioAndPct = getRecordNumerics(view, other.getRegionId(),
                            other.getAddress());
                    if (otherPrioAndPct != null) {
                        rrSrv.setPriority(otherPrioAndPct.getPriority());
                        rrSrv.setWeight(otherPrioAndPct.getWeight());
                        srvs.add(rrSrv);
                    }
                }
            }
        } else {
            if (prioAndPct != null) {
                DnsSrvRecord record = DnsSrvRecord.domainLevel(rrs.getProto(), rrs.getResource(), rr.getPort(),
                        rr.getAddress());
                record.setWeight(prioAndPct.getWeight());
                record.setPriority(prioAndPct.getPriority());
                record.setInternal(false);
                srvs.add(record);
            }
        }
        return srvs;
    }

    @Override
    @JsonIgnore
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DnsManager.FEATURE);
    }
}

