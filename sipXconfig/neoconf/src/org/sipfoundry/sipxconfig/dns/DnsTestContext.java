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

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.dns.DnsTestContextImpl.PrivateResourceRecord;
import org.sipfoundry.sipxconfig.region.Region;

public interface DnsTestContext {

    /**
     * return missing records if invalid
     *
     * @return
     */
    public String missingRecords(String server);

    String missingRecords(Region region, String server);

    Map<String, List<PrivateResourceRecord>> getComputedRecords(int regionId);
}
