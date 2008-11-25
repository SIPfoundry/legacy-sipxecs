/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * CustomDialingRule
 */
public class CustomDialingRule extends DialingRule {
    private static final String ROUTE_PATTERN = "route=%s";

    private List<DialPattern> m_dialPatterns = new ArrayList<DialPattern>();
    private CallPattern m_callPattern = new CallPattern();
    private List<String> m_permissionNames = new ArrayList<String>();

    public CustomDialingRule() {
        m_dialPatterns.add(new DialPattern());
    }

    @Override
    protected Object clone() throws CloneNotSupportedException {
        CustomDialingRule clone = (CustomDialingRule) super.clone();
        clone.m_permissionNames = new ArrayList(m_permissionNames);
        clone.m_dialPatterns = new ArrayList(m_dialPatterns);
        return clone;
    }

    public List<DialPattern> getDialPatterns() {
        return m_dialPatterns;
    }

    public void setDialPatterns(List<DialPattern> dialPaterns) {
        m_dialPatterns = dialPaterns;
    }

    public CallPattern getCallPattern() {
        return m_callPattern;
    }

    public void setCallPattern(CallPattern callPattern) {
        m_callPattern = callPattern;
    }

    @Override
    public String[] getPatterns() {
        String[] patterns = new String[m_dialPatterns.size()];
        for (int i = 0; i < patterns.length; i++) {
            DialPattern p = m_dialPatterns.get(i);
            patterns[i] = p.calculatePattern();
        }
        return patterns;
    }

    @Override
    public Transform[] getTransforms() {
        final String outPattern = getCallPattern().calculatePattern();
        List<Gateway> gateways = getGateways();
        Transform[] transforms;
        if (gateways.isEmpty()) {
            FullTransform transform = new FullTransform();
            transform.setUser(outPattern);
            if (getSchedule() != null) {
                String validTime = getSchedule().calculateValidTime();
                String scheduleParam = String.format(VALID_TIME_PARAM, validTime);
                transform.setFieldParams(scheduleParam);
            }
            transforms = new Transform[] {
                transform
            };
        } else {
            transforms = new Transform[gateways.size()];
            ForkQueueValue q = new ForkQueueValue(gateways.size());
            for (int i = 0; i < transforms.length; i++) {
                transforms[i] = getGatewayTransform(gateways.get(i), outPattern, q);
            }
        }
        return transforms;
    }

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
                Group s = g.getSite();
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
        List<Gateway> gateways = getGateways();
        for (Gateway g : gateways) {
            Group site = g.getSite();
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
        ArrayList<Transform> transforms = new ArrayList<Transform>(gateways.size());
        final String outPattern = getCallPattern().calculatePattern();
        ForkQueueValue q = new ForkQueueValue(gateways.size());
        for (Gateway g : gateways) {
            transforms.add(getGatewayTransform(g, outPattern, q));
        }
        return transforms;
    }

    private FullTransform getGatewayTransform(Gateway g, String pattern, ForkQueueValue q) {
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
        return transform;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.CUSTOM;
    }

    public void setPermissions(List<Permission> permissions) {
        List<String> permissionNames = getPermissionNames();
        permissionNames.clear();
        for (Permission permission : permissions) {
            permissionNames.add(permission.getName());
        }
    }

    public void setPermissionNames(List<String> permissionNames) {
        m_permissionNames = permissionNames;
    }

    @Override
    public List<String> getPermissionNames() {
        return m_permissionNames;
    }

    /**
     * External rule if there are gateways. Internal if no gateways
     */
    public boolean isInternal() {
        return getGateways().isEmpty();
    }

    public boolean isGatewayAware() {
        return true;
    }

    @Override
    public String[] getTransformedPatterns(Gateway gateway) {
        List<DialPattern> dialPatterns = getDialPatterns();
        Set<String> transformedPatterns = new LinkedHashSet<String>();
        for (DialPattern dp : dialPatterns) {
            DialPattern tdp = m_callPattern.transform(dp);
            if (gateway != null) {
                String pattern = gateway.getCallPattern(tdp.calculatePattern());
                transformedPatterns.add(pattern);
            } else {
                String pattern = tdp.calculatePattern();
                transformedPatterns.add(pattern);
            }
        }
        return transformedPatterns.toArray(new String[transformedPatterns.size()]);
    }
}
