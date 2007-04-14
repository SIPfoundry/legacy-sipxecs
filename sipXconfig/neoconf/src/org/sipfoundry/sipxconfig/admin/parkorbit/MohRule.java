/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.admin.dialplan.InternalForwardRule;

public class MohRule extends InternalForwardRule {

    public MohRule() {
        super("~~mh~", "<sip:moh@${ORBIT_SERVER_SIP_SRV_OR_HOSTPORT}>");
        setName("Music on Hold");
        setDescription("Forward music on hold calls to Park Server");
    }
}
