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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxServiceConfiguration;

public class PagingConfiguration extends SipxServiceConfiguration {

    private PagingContext m_pagingContext;

    private List<PagingGroup> m_pagingGroups;

    private String m_sipTraceLevel;

    private void generate() {
        PagingServer server = m_pagingContext.getPagingServer();
        List<PagingGroup> pagingGroups = m_pagingContext.getPagingGroups();
        m_pagingGroups = pagingGroups;
        m_sipTraceLevel = server.getSipTraceLevel();
    }

    @Override
    protected VelocityContext setupContext(Location location) {
        generate();
        VelocityContext context = super.setupContext(location);
        context.put("groups", m_pagingGroups);
        context.put("sipTraceLevel", m_sipTraceLevel);

        SipxPageService pageService = (SipxPageService) getService(SipxPageService.BEAN_ID);
        context.put("pageService", pageService);
        context.put("settings", pageService.getSettings().getSetting("page-config"));

        return context;
    }

    public void setPagingContext(PagingContext pagingContext) {
        m_pagingContext = pagingContext;
    }

    @Override
    public boolean isReplicable(Location location) {
        return getSipxServiceManager().isServiceInstalled(location.getId(), SipxPageService.BEAN_ID);
    }
}
