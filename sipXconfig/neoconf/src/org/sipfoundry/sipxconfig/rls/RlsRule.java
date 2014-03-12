/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.rls;

import org.sipfoundry.sipxconfig.dialplan.CallTag;

import org.sipfoundry.sipxconfig.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.dialplan.InternalForwardRule;
import org.sipfoundry.sipxconfig.dialplan.config.FullTransform;

public class RlsRule extends InternalForwardRule {
    private static final FullTransform RLS_TRANSFORM = new FullTransform();

    static {
        RLS_TRANSFORM.setHeaderParams("Route=sip:${RLS_SIP_SRV_OR_HOSTPORT};transport=tcp");
    }

    public RlsRule() {
        super(new DialPattern("~~rl~", DialPattern.VARIABLE_DIGITS), RLS_TRANSFORM);
        setName("RLS");
        setDescription("Forward resource list subscriptions to RLS");
    }

    @Override
    public CallTag getCallTag() {
        return CallTag.RL;
    }
}
