/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * LocationBasedDialingRule
 */
public abstract class LocationBasedDialingRule extends DialingRule {
    private static final String ROUTE_PATTERN = "route=%s";

    public abstract String getOutPattern();

    /**
     * Creates a data structure with a list of transforms for every site reference by this rule.
     * Shared gateways are included in every site list, but with a lower priority that gateways
     * specific for this site. Shared gateways and gateways without any site are also included in
     * a list associated with empty site (represented by an empty String)
     */
    @Override
    public Map<String, List<Transform>> getSiteTransforms() {
        Map<String, List<Gateway>> sitesToGateways = new LinkedHashMap<String, List<Gateway>>();
        List<Gateway> anywhereGateways = new ArrayList<Gateway>();
        sortGateways(sitesToGateways, anywhereGateways);

        Map<String, List<Transform>> sitesToTransforms = new LinkedHashMap<String, List<Transform>>();
        for (Map.Entry<String, List<Gateway>> entry : sitesToGateways.entrySet()) {
            String site = entry.getKey();
            List<Gateway> siteGateways = entry.getValue();
            // most anywhere gateways are good for any site - need to be re-added
            for (Gateway g : anywhereGateways) {
                Branch s = g.getBranch();
                if (s == null || !s.getName().equals(site)) {
                    siteGateways.add(g);
                }
            }
            List<Transform> transforms = calculateTransformsForGateways(siteGateways);
            sitesToTransforms.put(site, transforms);
        }
        if (!anywhereGateways.isEmpty()) {
            List<Transform> anywhereTransforms = calculateTransformsForGateways(anywhereGateways);
            sitesToTransforms.put(StringUtils.EMPTY, anywhereTransforms);
        }

        return sitesToTransforms;
    }

    private void sortGateways(Map<String, List<Gateway>> sitesToGateways, List<Gateway> anywhereGateways) {
        List<Gateway> gateways = getEnabledGateways();
        for (Gateway g : gateways) {
            Branch site = g.getBranch();
            if (site == null) {
                anywhereGateways.add(g);
                continue;
            }
            String siteName = site.getName();
            List<Gateway> gl = sitesToGateways.get(siteName);
            if (gl == null) {
                gl = new ArrayList<Gateway>();
                sitesToGateways.put(siteName, gl);
            }
            gl.add(g);
            if (g.isShared()) {
                anywhereGateways.add(g);
            }
        }
    }

    private List<Transform> calculateTransformsForGateways(List<Gateway> gateways) {
        final String pattern = getOutPattern();
        ArrayList<Transform> transforms = new ArrayList<Transform>(gateways.size());
        ForkQueueValue q = new ForkQueueValue(gateways.size());
        for (Gateway g : gateways) {
            transforms.add(getGatewayTransform(g, pattern, q));
        }
        return transforms;
    }

    protected FullTransform getGatewayTransform(Gateway g, String pattern, ForkQueueValue q) {
        FullTransform transform = new FullTransform();
        transform.setHost(g.getGatewayAddress());
        transform.setUser(g.getCallPattern(pattern));
        transform.addFieldParams(q.getSerial());
        if (getSchedule() != null) {
            String validTime = getSchedule().calculateValidTime();
            String scheduleParam = String.format(VALID_TIME_PARAM, validTime);
            transform.addFieldParams(scheduleParam);
        }
        String route = g.getRoute();
        if (StringUtils.isNotBlank(route)) {
            transform.setHeaderParams(String.format(ROUTE_PATTERN, route));
        }
        String transport = g.getGatewayTransportUrlParam();
        if (transport != null) {
            transform.setUrlParams(transport);
        }
        transform.addHeaderParams(String.format(GATEWAY_EXPIRES_PATTERN, GATEWAY_EXPIRES_VALUE));
        //transform.addHeaderParams(String.format(GATEWAY_LINEID_PATTERN, g.getId().toString()));
        transform.addUrlParams(String.format(GATEWAY_LINEID_PATTERN,  g.getId().toString()));
        return transform;
    }
}
