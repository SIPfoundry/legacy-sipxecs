/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.snmp;

import org.snmp4j.smi.VariableBinding;

public interface SnmpTableWalker {

    public void next(VariableBinding vb);

    public void up();
}
