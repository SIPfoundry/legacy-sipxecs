/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dns;

public interface DnsTestContext {

    /**
     * return missing records if invalid
     * @return
     */
    public String missingRecords(String server);
}
