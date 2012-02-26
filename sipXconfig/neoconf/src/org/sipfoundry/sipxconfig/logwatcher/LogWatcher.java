/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.logwatcher;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;

public interface LogWatcher {
    public static final GlobalFeature FEATURE = new GlobalFeature("sipxlogwatcher");
    public static final ProcessDefinition SERVICE = new ProcessDefinition("logwatcher",
            "/usr/bin/perl\\s+-w\\s+/usr/bin/sec");

    /**
     * Avoids checkstyle error
     */
    public void nop();
}
