/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.dialplan.config;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;

public abstract class RulesFile {
    private String m_domainName;
    private Location m_location;

    public abstract void write(Writer writer) throws IOException;

    /** called before generating rules */
    public abstract void begin();

    public abstract void generate(IDialingRule rule);

    /** called after last rule is generated */
    public abstract void end();

    public String getDomainName() {
        return m_domainName;
    }

    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }

    public Location getLocation() {
        return m_location;
    }

    public void setLocation(Location location) {
        m_location = location;
    }
}
