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
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * CustomDialingRule
 */
public class CustomDialingRule extends LocationBasedDialingRule {

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
        final String outPattern = getOutPattern();
        List<Gateway> gateways = getEnabledGateways();
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

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.CUSTOM;
    }

    @Override
    public CallTag getCallTag() {
        CallTag callTag = super.getCallTag();
        if (callTag != null) {
            return callTag;
        }
        return CallTag.CUST;
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

    @Override
    public String getOutPattern() {
        return m_callPattern.calculatePattern();
    }
}
