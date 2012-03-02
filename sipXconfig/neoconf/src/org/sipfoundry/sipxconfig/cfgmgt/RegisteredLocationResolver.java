/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.collections.Closure;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.csv.CsvParserImpl;
import org.sipfoundry.sipxconfig.commserver.Location;

/**
 * Determines what locations have ever got past setup. If not, there little reason to
 * trigger an agent run on the remote machines.  You should always update config though
 * for when it does complete setup.
 */
public class RegisteredLocationResolver {
    private static final Log LOG = LogFactory.getLog(RegisteredLocationResolver.class);
    private Set<String> m_registeredIps;
    private File m_csvFile;
    private ConfigCommands m_configCommands;

    public RegisteredLocationResolver(ConfigCommands configCommands, Set<String> registeredIps, File csvFile) {
        m_registeredIps = registeredIps;
        m_csvFile = csvFile;
        m_configCommands = configCommands;
    }

    public Collection<Location> getRegisteredLocations(Collection<Location> candidates) {
        boolean fresh = false;
        boolean registered;
        Collection<Location> copy = new ArrayList<Location>(candidates.size());
        for (Location l : candidates) {
            registered = false;
            if (l.isPrimary()) {
                registered = true;
            } else if (m_registeredIps != null && m_registeredIps.contains(l.getAddress())) {
                registered = true;
            } else {
                if (fresh) {
                    registered = false;
                } else {
                    m_configCommands.lastSeen();
                    m_registeredIps = loadRegisteredIps();
                    fresh = true;
                    registered = m_registeredIps.contains(l.getAddress());
                }
            }

            if (registered) {
                copy.add(l);
            } else {
                LOG.info(l.getName() + " is not registered");
            }
        }
        return copy;
    }

    Set<String> loadRegisteredIps() {
        final Set<String> ips = new HashSet<String>();
        if (!m_csvFile.exists()) {
            LOG.error("Cannot determine registered systems, missing file " + m_csvFile.getAbsolutePath());
            return ips;
        }
        FileReader r = null;
        try {
            r = new FileReader(m_csvFile);
            CsvParserImpl parser = new CsvParserImpl();
            parser.setSkipHeaderLine(false);
            Closure addRow = new Closure() {
                @Override
                public void execute(Object arg0) {
                    String[] data = ((String[]) arg0);
                    String ip = data[3].trim();
                    ips.add(ip);
                }
            };
            parser.parse(r, addRow);
            LOG.info("Loaded registered systems " + ips.toString());
        } catch (FileNotFoundException e) {
            LOG.error("Could not read lastseen CSV file", e);
        } finally {
            IOUtils.closeQuietly(r);
        }
        return ips;
    }

    public Set<String> getRegisteredIps() {
        return m_registeredIps;
    }
}
