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
    private static final String TOPOLOGY_XML = "topology.test.xml";
    
    private LocationsManager m_locationsManager;
    private File m_topologyFile;
    
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
        LOG.info("Deleting topology.xml after data migration");
        m_topologyFile.delete();
    }
    
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
    
    public void setConfigDirectory(String configDirectory) {
        m_topologyFile = new File(configDirectory, TOPOLOGY_XML);
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
            LOG.warn("Could not find the file " + TOPOLOGY_XML, e);
            return new Location[0];
        } catch (IOException e) {
            throw new RuntimeException(e);
        } catch (SAXException e) {
            throw new RuntimeException(e);
        }
    }
    
    /** Open an input stream on the topology file and return it */
    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        InputStream stream = new FileInputStream(m_topologyFile);
        return stream;
    }
    
    private static final class LocationDigester extends Digester {
        public static final String PATTERN = "topology/location";

        protected void initialize() {
            setValidating(false);
            setNamespaceAware(false);

            push(new ArrayList());
            addObjectCreate(PATTERN, Location.class);
            addSetProperties(PATTERN, "id", "name");
            String[] elementNames = {
                "replication_url", "agent_url", "sip_domain"
            };
            String[] propertyNames = {
                "replicationUrl", "processMonitorUrl", "sipDomain"
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
