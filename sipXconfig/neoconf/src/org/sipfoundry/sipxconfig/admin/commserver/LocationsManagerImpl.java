/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
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
import org.xml.sax.SAXException;

public class LocationsManagerImpl implements LocationsManager {
    private static final Log LOG = LogFactory.getLog(SipxReplicationContextImpl.class);

    private static final String TOPOLOGY_XML = "topology.xml";
    private String m_configDirectory;

    /** these are lazily constructed - always use accessors */
    private Location[] m_locations;

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    /** Return the replication URLs, retrieving them on demand */
    public synchronized Location[] getLocations() {
        if (m_locations == null) {
            m_locations = createLocations();
        }
        return m_locations;
    }

    private Location[] createLocations() {
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

    private static final class LocationDigester extends Digester {
        public static final String PATTERN = "topology/location";

        protected void initialize() {
            setValidating(false);
            setNamespaceAware(false);

            push(new ArrayList());
            addObjectCreate(PATTERN, Location.class);
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

    /** Open an input stream on the topology file and return it */
    protected InputStream getTopologyAsStream() throws FileNotFoundException {
        File file = new File(m_configDirectory, TOPOLOGY_XML);
        InputStream stream = new FileInputStream(file);
        return stream;
    }
}
