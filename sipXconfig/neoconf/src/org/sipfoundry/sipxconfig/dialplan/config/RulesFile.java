/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
