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
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class PagingConfiguration extends TemplateConfigurationFile {

    private String m_audioDirectory;

    private String m_domain;

    private List<PagingGroup> m_pagingGroups;

    private String m_logLevel;

    private String m_sipTraceLevel;

    public void generate(PagingServer server, List<PagingGroup> pagingGroups,
            String audioDirectory, String domain) {
        m_pagingGroups = pagingGroups;
        m_audioDirectory = audioDirectory;
        m_domain = domain;
        m_logLevel = server.getSipxServer().getPagingLogLevel();
        m_sipTraceLevel = server.getSipTraceLevel();
        // "sipxpage.properties.in" needs to be reloaded when used in Logging page
        server.getSipxServer().resetSettings();
    }

    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(null);
        context.put("groups", m_pagingGroups);
        context.put("audioDir", m_audioDirectory);
        context.put("domain", m_domain);
        context.put("logLevel", m_logLevel);
        context.put("sipTraceLevel", m_sipTraceLevel);
        return context;
    }
}
