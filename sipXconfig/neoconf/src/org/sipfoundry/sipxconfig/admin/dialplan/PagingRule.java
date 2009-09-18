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

public class PagingRule extends DialingRule {

    private final FullTransform m_transform;
    private final DialPattern m_dialPattern;

    public PagingRule(String prefix, String alertInfo) {
        setEnabled(true);
        m_dialPattern = new DialPattern(prefix, DialPattern.VARIABLE_DIGITS);
        m_transform = new FullTransform();
        m_transform.setUser("{vdigits}");
        m_transform.setHost("${PAGE_SERVER_ADDR}:${PAGE_SERVER_SIP_PORT}");
        m_transform.setUrlParams(new String[] {
            "Alert-info=" + alertInfo
        });
    }

    @Override
    public String getDescription() {
        return "Pager";
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
        return DialingRuleType.PAGING;
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }
}
