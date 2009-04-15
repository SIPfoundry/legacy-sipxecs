/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.admin.configdiag.ExternalCommand;

public interface DnsTestContext {
    public String getResult();
    public boolean isValid();
    public void execute(boolean provideDns);
    public ExternalCommand getCommand();
    public boolean isRunTestNeeded();

}
