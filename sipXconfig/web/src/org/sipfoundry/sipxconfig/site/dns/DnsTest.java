/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dns;

import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.dns.DnsSettings;
import org.sipfoundry.sipxconfig.dns.DnsTestContext;
import org.sipfoundry.sipxconfig.dns.DnsTestContextImpl.PrivateResourceRecord;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

public abstract class DnsTest extends BaseComponent implements PageBeginRenderListener {

    @Persist
    public abstract String getDnsServer();

    public abstract void setDnsServer(String dnsServer);

    @Persist
    public abstract String getResults();

    public abstract void setResults(String results);

    @Persist
    public abstract String getComputedRecords();

    public abstract void setComputedRecords(String tested);

    @Persist
    public abstract Integer getRegionId();

    public abstract void setRegionId(Integer id);

    @InjectObject(value = "spring:dnsTestContext")
    public abstract DnsTestContext getDnsTestContext();

    @InjectObject(value = "spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @InjectObject(value = "spring:dnsManager")
    public abstract DnsManager getDnsManager();

    public abstract void setShowDetailedHelp(boolean toggle);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public boolean isResultsNotBlank() {
        return StringUtils.isNotBlank(getResults());
    }

    public boolean isComputedNotBlank() {
        return StringUtils.isNotBlank(getComputedRecords());
    }

    @InjectObject(value = "spring:regionManager")
    public abstract RegionManager getRegionManager();

    @Override
    public void pageBeginRender(PageEvent event) {
        String dns = getDnsServer();
        if (StringUtils.isBlank(dns)) {
            // Try to select the most likely server an admin would want to
            // test, but leaving last tested server in http sessions will
            // also be useful
            DnsSettings settings = getDnsManager().getSettings();
            List<String> unmanaged = settings.getUnmanagedDnsServers();
            dns = (unmanaged != null && !unmanaged.isEmpty() ? unmanaged.get(0) : null);
            if (dns == null) {
                List<Location> l = getFeatureManager().getLocationsForEnabledFeature(DnsManager.FEATURE);
                dns = (l != null && !l.isEmpty() ? l.get(0).getAddress() : null);
            }
            setDnsServer(dns);
        }
        List<Region> regions = getRegionManager().getRegions();
        if (!regions.isEmpty()) {
            if (getRegionId() == null) {
                setRegionId(regions.get(0).getId());
            }
        }
    }

    public void execute() {
        String missingRecords = "";
        setComputedRecords(null);
        if (getRegionId() != null) {
            missingRecords = getDnsTestContext().missingRecords(getRegionManager().getRegion(getRegionId()),
                    getDnsServer());
            Map<String, List<PrivateResourceRecord>> computedRecords = getDnsTestContext().getComputedRecords(
                    getRegionId());
            StringBuilder computedResults = new StringBuilder();
            for (String key : computedRecords.keySet()) {
                for (PrivateResourceRecord prr : computedRecords.get(key)) {
                    computedResults.append(prr);
                }
            }
            setComputedRecords(computedResults.toString());

        } else {
            missingRecords = getDnsTestContext().missingRecords(getDnsServer());
        }
        setResults(missingRecords);
        if (StringUtils.isBlank(missingRecords)) {
            TapestryUtils.recordSuccess(this, getMessages().getMessage("validationDns.labelSuccess"));
        }
    }
}
