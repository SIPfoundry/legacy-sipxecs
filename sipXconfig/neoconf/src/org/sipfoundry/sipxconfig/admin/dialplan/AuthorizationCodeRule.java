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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

public class AuthorizationCodeRule extends DialingRule {

    private static final String USER_PART = "AUTH";
    private static final String COMMAND_PART = "command=";
    private final UrlTransform m_transform;
    private final DialPattern m_dialPattern;
    private int m_port;


    public AuthorizationCodeRule(String prefix, String addrPort, String operation) {
        setEnabled(true);
        m_dialPattern = new DialPattern(prefix, 0);
        m_transform = new UrlTransform();
        StringBuilder params = new StringBuilder();
        params.append(COMMAND_PART);
        params.append(operation);
        params.append(";");
        m_transform.setUrl(MappingRule.buildUrl(USER_PART, addrPort, params.toString(), null, null));
    }

    @Override
    public String getDescription() {
        return "Authorization Code";
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
        return DialingRuleType.AUTHCODE;
    }

    public boolean isInternal() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }
}
