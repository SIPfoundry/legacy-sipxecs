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
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * LongDistanceRule
 */
public class LocalRule extends DialingRule {
    private String m_pstnPrefix;
    private int m_externalLen;

    public LocalRule() {
        // empty
    }

    @Override
    public List<String> getPermissionNames() {
        List perms = Collections.singletonList(PermissionName.LOCAL_DIALING.getName());
        return perms;
    }

    @Override
    public String[] getPatterns() {
        DialPattern patternFull = new DialPattern(m_pstnPrefix, m_externalLen);
        DialPattern patternShort = new DialPattern(StringUtils.EMPTY, m_externalLen);

        return new String[] {
            patternFull.calculatePattern(), patternShort.calculatePattern()
        };
    }

    public String[] getTransformedPatterns() {
        DialPattern patternShort = new DialPattern(StringUtils.EMPTY, m_externalLen);
        return new String[] {
            patternShort.calculatePattern()
        };
    }

    @Override
    public Transform[] getTransforms() {
        CallPattern patternNormal = new CallPattern(StringUtils.EMPTY, CallDigits.VARIABLE_DIGITS);
        String user = patternNormal.calculatePattern();
        List<Gateway> gateways = getEnabledGateways();
        List<Transform> transforms = new ArrayList<Transform>(gateways.size());
        ForkQueueValue q = new ForkQueueValue(gateways.size());
        for (Gateway gateway : gateways) {
            FullTransform transform = new FullTransform();
            transform.setUser(gateway.getCallPattern(user));
            transform.setHost(gateway.getGatewayAddress());
            String transport = gateway.getGatewayTransportUrlParam();
            if (transport != null) {
                transform.setUrlParams(transport);
            }
            String[] fieldParams = new String[] {
                q.getSerial()
            };
            transform.setFieldParams(fieldParams);
            transforms.add(transform);
        }
        return transforms.toArray(new Transform[transforms.size()]);
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.LOCAL;
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    public boolean isGatewayAware() {
        return true;
    }

    public int getExternalLen() {
        return m_externalLen;
    }

    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
    }

    public String getPstnPrefix() {
        return m_pstnPrefix;
    }

    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
    }
}
