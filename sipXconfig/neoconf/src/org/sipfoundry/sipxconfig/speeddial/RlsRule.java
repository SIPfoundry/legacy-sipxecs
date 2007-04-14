/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalForwardRule;

public class RlsRule extends InternalForwardRule {
    public RlsRule() {
        super(new DialPattern("~~rl~", DialPattern.VARIABLE_DIGITS),
                "<sip:{digits}@${RLS_ADDR}:${RLS_SIP_PORT}>");
        setName("RLS");
        setDescription("Forward resource list subscriptions to RLS");
    }
}
