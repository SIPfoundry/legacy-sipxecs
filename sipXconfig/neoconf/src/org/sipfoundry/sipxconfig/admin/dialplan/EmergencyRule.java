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

import org.sipfoundry.sipxconfig.admin.dialplan.MediaServer.Operation;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * LongDistanceRule
 */
public class EmergencyRule extends DialingRule {
    private static final String SOS = "sos";

    private String m_emergencyNumber;
    private String m_optionalPrefix;
    private boolean m_useMediaServer;

    public String[] getPatterns() {
        ArrayList<String> patterns = new ArrayList<String>();
        patterns.add(SOS);
        patterns.add(m_emergencyNumber);
        if (null != m_optionalPrefix && 0 < m_optionalPrefix.length()) {
            patterns.add(m_optionalPrefix + m_emergencyNumber);
        }
        return patterns.toArray(new String[patterns.size()]);
    }

    public Transform[] getStandardTransforms() {
        List<Gateway> gateways = getGateways();
        if (gateways.size() <= 0) {
            return new Transform[0];
        }
        List<Transform> transforms = new ArrayList<Transform>(gateways.size());
        ForkQueueValue q = new ForkQueueValue(gateways.size());
        for (Gateway gateway : gateways) {
            FullTransform transform = new FullTransform();
            transform.setUser(gateway.getCallPattern(m_emergencyNumber));
            transform.setHost(gateway.getGatewayAddress());
            String transport = gateway.getGatewayTransportUrlParam();
            if (transport != null) {
                transform.setUrlParams(transport);
            }
            transform.addFieldParams(q.getSerial());
            if (getSchedule() != null) {
                String validTime = getSchedule().calculateValidTime();
                String scheduleParam = String.format(VALID_TIME_PARAM, validTime);
                transform.addFieldParams(scheduleParam);
            }
            transforms.add(transform);
        }
        return transforms.toArray(new Transform[transforms.size()]);
    }

    public Transform[] getMediaServerTransforms() {
        UrlTransform transform = new UrlTransform();
        MediaServer mediaServer = new SipXMediaServer();
        String url = MappingRule.buildUrl(CallDigits.FIXED_DIGITS, mediaServer, Operation.SOS,
                null);
        transform.setUrl(url);
        return new Transform[] {
            transform
        };
    }

    public Transform[] getTransforms() {
        return m_useMediaServer ? getMediaServerTransforms() : getStandardTransforms();
    }

    public DialingRuleType getType() {
        return DialingRuleType.EMERGENCY;
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        if (!m_useMediaServer) {
            super.appendToGenerationRules(rules);
            return;
        }
        try {
            DialingRule rule = (DialingRule) clone();
            rule.setGateways(Collections.<Gateway> emptyList());
            rule.setDescription(getDescription());
            rules.add(rule);
        } catch (CloneNotSupportedException e) {
            // should never happen
            throw new RuntimeException(e);
        }
    }

    public String getEmergencyNumber() {
        return m_emergencyNumber;
    }

    public void setEmergencyNumber(String emergencyNumber) {
        m_emergencyNumber = emergencyNumber;
    }

    public String getOptionalPrefix() {
        return m_optionalPrefix;
    }

    public void setOptionalPrefix(String optionalPrefix) {
        m_optionalPrefix = optionalPrefix;
    }

    public boolean getUseMediaServer() {
        return m_useMediaServer;
    }

    public void setUseMediaServer(boolean useMediaServer) {
        m_useMediaServer = useMediaServer;
    }
}
