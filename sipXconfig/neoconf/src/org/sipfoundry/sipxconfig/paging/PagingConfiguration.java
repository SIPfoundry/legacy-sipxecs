/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigFileType;

public class PagingConfiguration extends TemplateConfigurationFile {

    private String m_audioDirectory;

    private String m_domain;

    private List<PagingGroup> m_pagingGroups;

    private String m_logLevel;

    public void generate(PagingServer server, List<PagingGroup> pagingGroups,
            String audioDirectory, String domain) {
        m_pagingGroups = pagingGroups;
        m_audioDirectory = audioDirectory;
        m_domain = domain;
        m_logLevel = server.getLogLevel();
    }

    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        context.put("groups", m_pagingGroups);
        context.put("audioDir", m_audioDirectory);
        context.put("domain", m_domain);
        context.put("logLevel", m_logLevel);
        return context;
    }

    public ConfigFileType getType() {
        return ConfigFileType.PAGING_CONFIG;
    }
}
