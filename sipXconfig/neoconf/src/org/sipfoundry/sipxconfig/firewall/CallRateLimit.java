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
package org.sipfoundry.sipxconfig.firewall;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class CallRateLimit extends BeanWithId implements Serializable {
    private int m_rate;
    private String m_sipMethod;
    private String m_interval;
    private CallRateRule m_rule;

    public enum SipMethod {
        INVITE, REGISTER, OPTIONS, ACK, SUBSCRIBE;

        public static SipMethod getSipMethod(String sipMethod) {
            if (sipMethod == null) {
                return null;
            }
            try {
                return valueOf(sipMethod);
            } catch (IllegalArgumentException e) {
                return null;
            }
        }
    };

    public enum CallRateInterval {
        second, minute, hour, day;

        public static CallRateInterval getCallRateInterval(String interval) {
            if (interval == null) {
                return CallRateInterval.second;
            }
            try {
                return valueOf(interval);
            } catch (IllegalArgumentException e) {
                return CallRateInterval.second;
            }
        }
    };

    public void setRate(int rate) {
        m_rate = rate;
    }

    public void setSipMethod(String method) {
        m_sipMethod = method;
    }

    public void setSipMethodId(SipMethod method) {
        m_sipMethod = method.toString();
    }

    public void setInterval(String interval) {
        m_interval = interval;
    }

    public void setIntervalId(CallRateInterval interval) {
        m_interval = interval.toString();
    }

    public void setCallRateRule(CallRateRule rule) {
        m_rule = rule;
    }

    public int getRate() {
        return m_rate;
    }

    public String getSipMethod() {
        return m_sipMethod;
    }

    public SipMethod getSipMethodId() {
        return SipMethod.getSipMethod(m_sipMethod);
    }

    public String getInterval() {
        return m_interval;
    }

    public CallRateInterval getIntervalId() {
        return CallRateInterval.getCallRateInterval(m_interval);
    }

    public CallRateRule getCallRateRule() {
        return m_rule;
    }

}
