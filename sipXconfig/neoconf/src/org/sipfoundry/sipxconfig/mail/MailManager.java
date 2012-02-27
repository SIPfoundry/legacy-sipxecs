/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.mail;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;

public interface MailManager {
    public static final GlobalFeature FEATURE = new GlobalFeature("mail");
    public static final ProcessDefinition SERVICE = new ProcessDefinition("sendmail");

    /** avoids checkstyle error */
    public void nop();

}
