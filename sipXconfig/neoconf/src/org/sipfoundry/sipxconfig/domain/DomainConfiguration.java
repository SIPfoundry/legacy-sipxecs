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
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;

public class DomainConfiguration extends TemplateConfigurationFile {

    private Domain m_domain;
    private String m_realm;
    private String m_language;

    public void generate(Domain domain, String realm, String language) {
        m_domain = domain;
        m_realm = realm;
        m_language = language;
    }

    @Override
    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        context.put("domain", m_domain);
        context.put("realm", m_realm);
        context.put("language", m_language);
        return context;
    }

    public ConfigFileType getType() {
        return ConfigFileType.DOMAIN_CONFIG;
    }
}
