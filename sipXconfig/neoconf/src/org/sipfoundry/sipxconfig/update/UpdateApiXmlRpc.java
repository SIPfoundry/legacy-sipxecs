/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpException;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SoftwareAdminApi;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

import static org.apache.commons.lang.StringUtils.split;

/**
 * Implementation of update API based on XML/RPC methods provided by sipXsupervisor
 */
public class UpdateApiXmlRpc implements UpdateApi {
    private static final String GET_VERSION = "version";
    private static final String INSTALL_UPDATES = "update";
    private static final String CHECK_UPDATE = "check-update";
    private static final String VERSION_STRING = "version:";

    private static final Log LOG = LogFactory.getLog(UpdateApiXmlRpc.class);

    private LocationsManager m_locationsManager;

    private ApiProvider<SoftwareAdminApi> m_softwareAdminApiProvider;

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setSoftwareAdminApiProvider(ApiProvider<SoftwareAdminApi> softwareAdminApiProvider) {
        m_softwareAdminApiProvider = softwareAdminApiProvider;
    }

    public String getCurrentVersion() {
        // sipx-swadmin.py returns either:
        //    "Could not determine version" or
        //    "version:" + sipxPackage[0].name + " " + sipxPackage[0].ver + "-" + sipxPackage[0].release
        // We want to display only the version and release.
        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        SoftwareAdminApi api = getApi(primaryLocation);
        List<String> streams = api.exec(primaryLocation.getFqdn(), GET_VERSION);
        if (streams.isEmpty()) {
            return VERSION_NOT_DETERMINED;
        }
        String versionFilePath = streams.get(0);
        List<String> remoteFileLines = getResult(primaryLocation, api, GET_VERSION, versionFilePath);
        if (remoteFileLines.isEmpty()) {
            return VERSION_NOT_DETERMINED;
        }
        for (String line : remoteFileLines) {
            if (line != null && line.startsWith(VERSION_STRING)) {
                String[] items = split(line);
                if (items != null && items.length >= 2) {
                    return items[1];
                }
            }
        }
        return VERSION_NOT_DETERMINED;
    }

    public List<PackageUpdate> getAvailableUpdates() {
        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        SoftwareAdminApi api = getApi(primaryLocation);
        List<String> streams = api.exec(primaryLocation.getFqdn(), CHECK_UPDATE);
        if (streams.size() <= 0) {
            return Collections.emptyList();
        }
        List<PackageUpdate> updates = new ArrayList<PackageUpdate>();
        String updatesFilePath = streams.get(0);
        List<String> updateLines = getResult(primaryLocation, api, CHECK_UPDATE, updatesFilePath);
        for (String line : updateLines) {
            PackageUpdate packageUpdate = PackageUpdate.parse(line);
            if (packageUpdate != null) {
                updates.add(packageUpdate);
            }
        }
        return updates;
    }

    public void installUpdates() {
        Location primaryLocation = m_locationsManager.getPrimaryLocation();
        SoftwareAdminApi api = null;

        Location[] locations = m_locationsManager.getLocations();
        for (Location location : locations) {
            if (!location.isPrimary()) {
                api = getApi(location);
                api.exec(primaryLocation.getFqdn(), INSTALL_UPDATES);
            }
        }

        api = getApi(primaryLocation);
        api.exec(primaryLocation.getFqdn(), INSTALL_UPDATES);
    }

    /**
     * XML/RPC API is asynchronous: we need to actively poll the server to figure out when it
     * succeeds
     */
    private void waitForExecFinished(SoftwareAdminApi api, Location location, String command) {
        while (api.execStatus(location.getFqdn(), command).equals("RUNNING")) {
            try {
                Thread.sleep(5000);
            } catch (InterruptedException e) {
                LOG.error("Exception while waiting for XML-RPC command to finish.", e);
            }
        }
    }

    /**
     * Waits for XML/RPC method to complete and retrieves resulting file.
     */
    private List<String> getResult(Location location, SoftwareAdminApi api, String command, String path) {
        waitForExecFinished(api, location, command);
        String fileUrl = location.getHttpsServerUrl() + path;
        return retrieveRemoteFile(fileUrl);
    }

    /**
     * Retrieves the file from provided URL
     *
     * FIXME: this method does not belong here, it's a generic utility
     */
    protected List<String> retrieveRemoteFile(String fileUrl) {
        InputStream responseStream = null;
        BufferedReader responseReader = null;
        List<String> lines = new ArrayList<String>();
        GetMethod method = new GetMethod(fileUrl);

        try {
            HttpClient client = new HttpClient();
            int statusCode = client.executeMethod(method);
            if (statusCode != HttpStatus.SC_OK) {
                LOG.error("HTTP GET failed: " + method.getStatusLine());
            }
            responseStream = method.getResponseBodyAsStream();
            responseReader = new BufferedReader(new InputStreamReader(responseStream, method.getResponseCharSet()));
            String line = null;
            while ((line = responseReader.readLine()) != null) {
                lines.add(line);
            }
        } catch (HttpException e) {
            LOG.error("HTTP error", e);
        } catch (IOException e) {
            LOG.error("Connectivity error", e);
        } finally {
            IOUtils.closeQuietly(responseReader);
            IOUtils.closeQuietly(responseStream);
            method.releaseConnection();
        }
        return lines;
    }

    private SoftwareAdminApi getApi(Location location) {
        return m_softwareAdminApiProvider.getApi(location.getProcessMonitorUrl());
    }
}
