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

import org.sipfoundry.sipxconfig.api.RestServerApi;
import org.sipfoundry.sipxconfig.restserver.RestServer;
import org.sipfoundry.sipxconfig.restserver.RestServerSettings;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;

public class RestServerApiImpl extends BaseServiceApiImpl implements RestServerApi {
    private RestServer m_rest;

    public void setRestServer(RestServer server) {
        m_rest = server;
    }

    @Override
    protected PersistableSettings getSettings() {
        return m_rest.getSettings();
    }

    @Override
    protected void saveSettings(PersistableSettings settings) {
        m_rest.saveSettings((RestServerSettings) settings);
    }

}
