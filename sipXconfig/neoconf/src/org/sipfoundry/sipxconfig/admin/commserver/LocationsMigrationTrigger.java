/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.SetNestedPropertiesRule;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.xml.sax.SAXException;

public class LocationsMigrationTrigger extends InitTaskListener {
    private static final Log LOG = LogFactory.getLog(LocationsMigrationTrigger.class);

    private LocationsManager m_locationsManager;
    private String m_hostname;
    private String m_localIpAddress;
    private String m_topologyFilename;
    private String m_configDirectory;

    @Override
    public void onInitTask(String task) {
        // only do migration if there aren't already locations stored in database
        if (m_locationsManager.getLocations().length > 0) {
            return;
        }

        LOG.info("Migrating location data from topology.xml to sipXconfig database");
        Location[] locations = loadLocationsFromFile();
        for (Location location : locations) {
            m_locationsManager.storeLocation(location);
        }

        if (locations.length == 0) {
            LOG.info("No locations migrated from topology.xml - Creating localhost location.");
            Location localhostLocation = new Location();
            localhostLocation.setName("Config Server, Media Server and Comm Server");
            localhostLocation.setAddress(m_localIpAddress);
            localhostLocation.setFqdn(m_hostname);
            m_locationsManager.storeLocation(localhostLocation);
        }

        LOG.info("Deleting topology.xml after data migration");
        getTopologyFile().delete();
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void setHostname(String hostname) {
        m_hostname = hostname;
    }

    public void setLocalIpAddress(String localIpAddress) {
        m_localIpAddress = localIpAddress;
    }

    public void setTopologyFilename(String topologyFilename) {
        m_topologyFilename = topologyFilename;
    }

    private Location[] loadLocationsFromFile() {
        try {
            InputStream stream = getTopologyAsStream();
            Digester digester = new LocationDigester();
            Collection<Location> locations = (Collection) digester.parse(stream);
            return locations.toArray(new Location[locations.size()]);
        } catch (FileNotFoundException e) {
            // When running in a test environment, the topology file will not be found
            // set to empty array so that we do not have to parse again
            LOG.warn("Could not find the file " + m_topologyFilename, e);
            return new Location[0];
        } catch (IOException e) {
            LOG.warn("Error accessing file " + m_topologyFilename, e);
            return new Location[0];
        } catch (SAXException e) {
            LOG.warn("Error parsing file " + m_topologyFilename, e);
            return new Location[0];
        }
    }

    /** Open an input stream on the topology file and return it */
    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        InputStream stream = new FileInputStream(getTopologyFile());
        return stream;
    }

    private File getTopologyFile() {
        return new File(m_configDirectory, m_topologyFilename);
    }

    private static final class LocationDigester extends Digester {
        public static final String PATTERN = "topology/location";

        @Override
        protected void initialize() {
            setValidating(false);
            setNamespaceAware(false);

            push(new ArrayList());
            addObjectCreate(PATTERN, Location.class);
            addSetProperties(PATTERN, "id", "name");
            String[] elementNames = {
                "replication_url"
            };
            String[] propertyNames = {
                "url"
            };
            addSetProperties(PATTERN);
            SetNestedPropertiesRule rule = new SetNestedPropertiesRule(elementNames, propertyNames);
            // ignore all properties that we are not interested in
            rule.setAllowUnknownChildElements(true);
            addRule(PATTERN, rule);
            addSetNext(PATTERN, "add");
        }
    }
}
