/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ProcessManagerApi;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.annotation.Required;

public class ConfigVersionManagerImpl implements ConfigVersionManager {
    private static final Log LOG = LogFactory.getLog(ConfigVersionManagerImpl.class);

    private ApiProvider<ProcessManagerApi> m_processManagerApiProvider;

    private LocationsManager m_locationsManager;

    public void setConfigVersion(SipxService service, Location location) {
        VersionInfo versionInfo = new VersionInfo();
        String version = versionInfo.getVersion();
        setConfigurationVersionWorker(service, location, version);
    }

    private void setConfigurationVersionWorker(SipxService service, Location location, String version) {
        ProcessManagerApi api = m_processManagerApiProvider.getApi(location.getProcessMonitorUrl());
        try {
            String host = m_locationsManager.getPrimaryLocation().getFqdn();
            api.setConfigVersion(host, service.getProcessName(), version);
        } catch (XmlRpcRemoteException e) {
            LOG.error("Failed to update config version", e);
        }
    }

    @Required
    public void setProcessManagerApiProvider(ApiProvider<ProcessManagerApi> processManagerApiProvider) {
        m_processManagerApiProvider = processManagerApiProvider;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
