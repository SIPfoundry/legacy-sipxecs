/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.api.impl;

import javax.ws.rs.core.Response;

import org.sipfoundry.sipxconfig.api.DnsApi;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.dns.DnsSettings;
import org.sipfoundry.sipxconfig.dns.DnsTestContext;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;

public class DnsApiImpl extends BaseServiceApiImpl implements DnsApi {
    private DnsManager m_manager;
    private DnsTestContext m_advisorManager;

    @Override
    public Response getTestAdvisor(String serverId) {
        return Response.ok().entity(m_advisorManager.missingRecords(serverId)).build();
    }

    @Override
    protected PersistableSettings getSettings() {
        return m_manager.getSettings();
    }

    @Override
    protected void saveSettings(PersistableSettings settings) {
        m_manager.saveSettings((DnsSettings) settings);
    }

    public void setDnsManager(DnsManager manager) {
        m_manager = manager;
    }

    public void setDnsTestContext(DnsTestContext context) {
        m_advisorManager = context;
    }

}
