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
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Properties;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.SetNestedPropertiesRule;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.xml.sax.SAXException;

public class LocationsMigrationTrigger extends InitTaskListener {
    private static final Log LOG = LogFactory.getLog(LocationsMigrationTrigger.class);

    private LocationsManager m_locationsManager;
    private String m_topologyFilename;
    private String m_configDirectory;
    private String m_networkPropertiesFilename;

    @Override
    public void onInitTask(String task) {
        Location[] existingLocations = m_locationsManager.getLocations();
        if (existingLocations.length > 0) {
            // only do migration if there aren't already locations stored in database
            return;
        }

        LOG.info("Migrating location data from topology.xml to sipXconfig database");
        Location[] locations = loadLocationsFromFile();
        // if there is no location matching the sipxconfig hostname - the first found is made
        // primary
        if (locations.length > 0) {
            changeToPrimary(locations);
            // save locations in DB
            for (Location location : locations) {
                location.setRegistered(true);
                if (location.isPrimary()) {
                    location.setAddress(getAddressForPrimary(location.getFqdn()));
                }
                // save locations in DB without publishing events
                m_locationsManager.storeMigratedLocation(location);
            }
        } else {
            LOG.info("No locations migrated from topology.xml - Creating localhost location.");
            Location primaryLocation = new Location();
            primaryLocation.setName("Primary server");
            primaryLocation.setPrimary(true);
            primaryLocation.setRegistered(true);
            String primaryLocationFqdn = getFqdnForPrimaryLocation();
            primaryLocation.setFqdn(primaryLocationFqdn);
            primaryLocation.setAddress(getAddressForPrimary(primaryLocationFqdn));
            // save locations in DB without publishing events
            m_locationsManager.storeMigratedLocation(primaryLocation);
        }

        LOG.info("Deleting topology.xml after data migration");
        getTopologyFile().delete();
    }

    /**
     * Iterate through locations list to make sure that there is one and only one location primary
     * (the one that matches the hostname or the first in the list)
     */
    private void changeToPrimary(Location[] locations) {
        if (locations == null || locations.length == 0) {
            return;
        }

        Location firstLocation = locations[0];
        firstLocation.setPrimary(true);
        // always one and only one location should be designated as primary
        boolean savePrimary = false;
        for (Location location : locations) {
            if (location.getFqdn().equals(getFqdnForPrimaryLocation()) && !savePrimary) {
                firstLocation.setPrimary(false);
                location.setPrimary(true);
                savePrimary = true;
            }
        }
    }

    private String getFqdnForPrimaryLocation() {
        return System.getProperty("sipxconfig.hostname");
    }

    private String getAddressForPrimary(String primaryLocationFqdn) {
        File networkPropertiesFile = new File(m_configDirectory, m_networkPropertiesFilename);
        String primaryLocationAddress = StringUtils.EMPTY;
        try {
            Properties networkProperties = new Properties();
            InputStream inputStream = new FileInputStream(networkPropertiesFile);
            networkProperties.load(inputStream);
            primaryLocationAddress = networkProperties.getProperty("IpAddress");
        } catch (FileNotFoundException fnfe) {
            LOG.warn("Unable to find network properties file " + networkPropertiesFile.getPath()
                    + "  ( - Only an issue for new installation)");
        } catch (IOException ioe) {
            LOG.warn("Unable to load network properties file " + networkPropertiesFile.getPath()
                    + " ( - Only an issue for new installation)");
        }

        if (StringUtils.isEmpty(primaryLocationAddress)) {
            try {
                InetAddress primaryAddress = InetAddress.getByName(primaryLocationFqdn);
                primaryLocationAddress = primaryAddress.getHostAddress();
            } catch (UnknownHostException uhe) {
                LOG.warn("Unable to resolve address for " + primaryLocationFqdn);
            }
        }

        return primaryLocationAddress;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void setTopologyFilename(String topologyFilename) {
        m_topologyFilename = topologyFilename;
    }

    public void setNetworkPropertiesFilename(String networkPropertiesFilename) {
        m_networkPropertiesFilename = networkPropertiesFilename;
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
            LOG.warn("Could not find the file " + m_topologyFilename);
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
            SetNestedPropertiesRule rule = new SetNestedPropertiesRule(elementNames,
                    propertyNames);
            // ignore all properties that we are not interested in
            rule.setAllowUnknownChildElements(true);
            addRule(PATTERN, rule);
            addSetNext(PATTERN, "add");
        }
    }
}
