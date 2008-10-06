/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class DomainConfiguration extends TemplateConfigurationFile {

    private Domain m_domain;
    private String m_realm;
    private String m_language;
    private String m_alarmServerUrl;

    public void generate(Domain domain, String realm, String language, String alarmServerUrl) {
        m_domain = domain;
        m_realm = realm;
        m_language = language;
        m_alarmServerUrl = alarmServerUrl;
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("domain", m_domain);
        context.put("realm", m_realm);
        context.put("language", m_language);
        context.put("alarmServerUrl", m_alarmServerUrl);
        return context;
    }
}
