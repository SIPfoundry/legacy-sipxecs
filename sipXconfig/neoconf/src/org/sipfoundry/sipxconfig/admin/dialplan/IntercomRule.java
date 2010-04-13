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

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;

public class IntercomRule extends DialingRule {

    private final DialPattern m_dialPattern;
    private final FullTransform m_transform;

    public IntercomRule(boolean enabled, String prefix, String code, int timeout) {
        setEnabled(enabled);

        m_dialPattern = new DialPattern(prefix, DialPattern.VARIABLE_DIGITS);

        m_transform = new FullTransform();
        CallPattern callPattern = new CallPattern("", CallDigits.VARIABLE_DIGITS);
        String user = callPattern.calculatePattern();
        m_transform.setUser(user);
        String alertInto = String.format("Alert-info=%s", code);
        String callInfo = String.format("Call-Info=<sip:localhost>;answer-after=%d", timeout);
        m_transform.setHeaderParams(alertInto, callInfo);
    }

    public IntercomRule(Intercom intercom) {
        this(intercom.isEnabled(), intercom.getPrefix(), intercom.getCode(), intercom
                .getTimeoutInSeconds());
    }

    @Override
    public String[] getPatterns() {
        return new String[] {
            m_dialPattern.calculatePattern()
        };
    }

    @Override
    public Transform[] getTransforms() {
        return new Transform[] {
            m_transform
        };
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.INTERCOM;
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }
}
